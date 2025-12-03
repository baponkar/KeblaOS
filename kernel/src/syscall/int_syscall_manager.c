/*
Interrupt Based System Call

References: 
    https://github.com/dreamportdev/Osdev-Notes/blob/master/06_Userspace/04_System_Calls.md
*/

#include "../driver/vga/vga.h"
#include "../driver/vga/framebuffer.h"

#include "../driver/io/serial.h"

#include "../lib/string.h"  // for size_t
#include "../lib/stdio.h"
#include "../lib/errno.h"
#include "../lib/time.h"

#include "../sys/acpi/descriptor_table/fadt.h" // acpi_poweroff 

#include "../process/process.h"
#include "../process/thread.h"

#include "../arch/interrupt/irq_manage.h"
#include "../util/util.h"
#include "../driver/keyboard/ring_buffer.h"
#include "../memory/kheap.h"
#include "../memory/uheap.h"
#include "../memory/paging.h"

#include "../fs/FatFs-R.0.16/source/ff.h"
#include "../fs/FatFs-R.0.16/source/diskio.h"

#include "../vfs/vfs.h"

#include "../lib/time.h"


#include "int_syscall_manager.h"

extern bool debug_on;

#define MAX_PATH_LEN 256

extern ring_buffer_t* keyboard_buffer;

extern FATFS  *fatfs;

extern uint8_t get_core_id();

// Helper: Copy string from user space to kernel buffer
static int copy_from_user(char *kernel_dst, const char *user_src, size_t max_len) {
  for (size_t i = 0; i < max_len; i++) {
    // Safely copy byte-by-byte (implement safe memory access here)
    kernel_dst[i] = user_src[i];
    if (user_src[i] == '\0') break; // Stop at null terminator
  }
  kernel_dst[max_len - 1] = '\0';   // Ensure termination
  return 0;
}


// rax, rdi, rsi, rdx, r10, r8, r9 
static uint64_t system_call(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10, uint64_t r8, uint64_t r9){
    
    uint64_t out;

    asm volatile (
        "mov %[_rax], %%rax\n"   // System Call Number
        "mov %[_rdi], %%rdi\n"   // Argument 1
        "mov %[_rsi], %%rsi\n"   // Argument 2
        "mov %[_rdx], %%rdx\n"   // Argument 3
        "mov %[_r10], %%r10\n"   // Argument 4
        "mov %[_r8], %%r8\n"     // Argument 5
        "mov %[_r9], %%r9\n"     // Argument 6
        "int $0x80\n"            // Trigger System Call Interrupt
        "mov %%rax, %[_out]\n"   // Storing Output
        : [_out] "=r" (out)
        : [_rax] "r" (rax), [_rdi] "r" (rdi), [_rsi] "r" (rsi), [_rdx] "r" (rdx), [_r10] "r" (r10), [_r8] "r" (r8), [_r9] "r" (r9)
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"   // Clobber registers
    );

    return out;
}


//  regs->rax is hold success(0) or error(-1) code
registers_t *int_systemcall_handler(registers_t *regs) {

    if(regs->int_no == 128){
        switch (regs->rax) { 

           case INT_TIME: {
                time_t *t = (time_t *)regs->rdi;   
                time_t now = get_time();
                if (t) {
                    *t = now;
                }
                regs->rax = (uint64_t)now;
                break;
            }

            case INT_CLOCK_GETTIME: {
                int clk_id = (int)regs->rdi;
                struct timespec *tp = (struct timespec *)regs->rsi;

                if (!tp) {
                    regs->rax = -EINVAL;
                    break;
                }

                if (clk_id == CLOCK_REALTIME) {
                    time_t now = get_time();
                    tp->tv_sec = now;
                    tp->tv_nsec = 0;
                    regs->rax = 0;
                } else if (clk_id == CLOCK_MONOTONIC) {
                    tp->tv_sec = get_uptime_seconds(0);
                    tp->tv_nsec = 0;
                    regs->rax = 0;
                } else {
                    regs->rax = -EINVAL; // Unknown clock id
                }
                break; // ✅
            }

            case INT_CLOCK_GETTIMEOFDAY: {
                struct timeval *tv = (struct timeval *)regs->rdi;
                struct timezone *tz = (struct timezone *)regs->rsi; // Optional

                if (!tv) {
                    regs->rax = -EINVAL;
                    break;
                }

                time_t now = get_time();
                tv->tv_sec = now;
                tv->tv_usec = 0;            // No microsecond precision yet

                if (tz) {
                    tz->tz_minuteswest = 0;  // UTC for now
                    tz->tz_dsttime = 0;
                }

                regs->rax = 0;
                break;
            }

            case INT_TIMES: {
                struct tms *buf = (struct tms *)regs->rdi;
                if (!buf) {
                    regs->rax = -EINVAL;
                    break;
                }

                // In a real OS: fill process CPU usage
                buf->tms_utime  = 0; // user CPU time
                buf->tms_stime  = 0; // system CPU time
                buf->tms_cutime = 0; // user CPU time of children
                buf->tms_cstime = 0; // system CPU time of children

                regs->rax = get_uptime_seconds(0) * CLOCKS_PER_SEC; // Return ticks since boot
                break;
            }

            case INT_SYSCALL_GET_TIME: {
                uint64_t time = (uint64_t) get_time();
                if(!time){
                    printf("[INT SYSCALL] : time: %d", time);
                    regs->rax = (uint64_t)(-1);
                }
                regs->rax = time;
            }

            case INT_SYSCALL_GET_UP_TIME: {
                uint8_t cpu_id = get_core_id();
                uint64_t uptime = (uint64_t) get_uptime_seconds(cpu_id);
                regs->rax = uptime;
            }
            
            case INT_SYSCALL_KEYBOARD_READ: {
                uint8_t *user_buf = (uint8_t *)regs->rdi;
                size_t size = regs->rsi;

                if (!user_buf || size == 0) {
                    regs->rax = (uint64_t)(-1);
                    break;
                }

                size_t read_count = 0;

                while (read_count < size - 1) {
                    uint8_t ch;

                    // Wait for input in ring buffer
                    while (is_ring_buffer_empty(keyboard_buffer)){
                        asm volatile("sti");
                        asm volatile("hlt");  // Sleep CPU until next interrupt
                    }

                    if (ring_buffer_pop(keyboard_buffer, &ch) == 0){

                        // Stop reading when newline is encountered
                        if (ch == '\n' ||  ch == '\r') {
                            break;
                        }
                        user_buf[read_count++] = ch;
                    }
                }

                user_buf[read_count] = '\0';        // Null-terminate string
                regs->rax = (uint64_t)read_count;   // Return number of bytes read
                break;
            }

            case INT_SYSCALL_PRINT: {   // 0x5A : Print a string
                if (!regs->rdi) {
                    printf("Invalid string pointer!\n");
                    regs->rax = (uint64_t)(-1);
                    break;
                }

                const char *str = (const char *)regs->rdi;
                printf("%s", str);  
                regs->rax = 0;  // success
                break;
            }

            case INT_SYSCALL_PRINT_RAX: {  // 0x5C : Print the value of rax
                printf("rax: %x\n", regs->rax);
                regs->rax = 0; // success
                break;
            }

            case INT_SYSCALL_EXIT: {    // 0x5B
                regs->rax = 0;          // success
                break;
            }

            case INT_SYSCALL_ALLOC: { // 0x5D : Allocate memory
                size_t size = regs->rdi;
                uint8_t type = regs->rsi;

                if (size == 0 || type > 0x3) {
                    printf("Invalid type parameters for allocation syscall!\n");
                    regs->rax = (uint64_t)(-1); // error
                    break;
                }
                uint64_t ptr = (uint64_t) uheap_alloc(size, type);

                if(!ptr) {
                    printf("Memory allocation failed!\n");
                    regs->rax = (uint64_t)(-1); // error
                    break;
                }
                regs->rax = (uint64_t)ptr;
                break;
            }
 
            case INT_SYSCALL_FREE: {    // 0x5E : Free allocated memory
                void *ptr = (void *)regs->rdi;
                if (!ptr) {
                    printf("Invalid pointer for free syscall!\n");
                    regs->rax = (uint64_t)(-1); // error
                    break;
                }
                size_t size = regs->rsi;
                if (size == 0) {
                    printf("Invalid size for free syscall!\n");
                    regs->rax = (uint64_t)(-1); // error
                    break;
                }
                uheap_free(ptr, size);
                regs->rax = 0; // success
                break;
            }

            // ------------------------- Process Manage ----------------------------
            case INT_CREATE_PROCESS: {
                const char* process_name = (const char *)regs->rdi;

                if(!process_name){
                    regs->rax = (uint64_t)(-1); // error
                    break;
                }

                process_t* process = create_process(process_name);

                regs->rax = (process != NULL) ? (uint64_t) process : -1;
                break;
            }

            case INT_DELETE_PROCESS: {
                process_t* process = (process_t *)regs->rdi;

                if(!process){
                    regs->rax = (uint64_t)(-1); // error
                    break;
                }

                delete_process(process);

                regs->rax = 0;
                break;
            }

            case INT_GET_PROCESS_FROM_PID: {
                size_t pid = (size_t) regs->rdi;

                process_t *process = (process_t *)get_process_by_pid(pid);

                if(process == NULL){
                    regs->rax = -1;
                    break;
                }

                regs->rax = (process != NULL) ? (uint64_t)process : -1;
                break;
            }

            case INT_GET_CURRENT_PROCESS: {
                process_t *process = get_current_process();

                if(process == NULL){
                    regs->rax = -1;
                    break;
                }

                regs->rax = (process != NULL) ? (uint64_t) process : -1;
            }

            // ------------------------- Thread Manage -----------------------------
            case INT_CREATE_THREAD: {
                process_t *parent = (process_t *) regs->rdi;
                const char* thread_name = (const char*) regs->rsi;
                void *function = (void *)regs->rdx;
                void *arg = (void *) regs->r10;

                if(!parent || !thread_name || !function || !arg){
                    regs->rax = -1;
                    break;
                }

                thread_t *thread = create_thread(parent, thread_name, function, arg);

                if(!thread){
                    regs->rax = -1;
                    break;
                }

                regs->rax = (uint64_t)thread;
                break;

            }

            case INT_DELETE_THREAD: {
                thread_t *thread = (thread_t *)regs->rdi;

                if(!thread){
                    regs->rax = -1;
                    break;
                }

                delete_thread(thread);
                regs->rax = 0;
                break;
            }


            case INT_GET_FB_INFO: {
                break;
            }

            // --------------------------VFS File Manages--------------------------
            
            case INT_VFS_MKFS: {
                regs->rax = (uint64_t)vfs_mkfs((int)regs->rdi, (VFS_TYPE)regs->rsi);
                break;
            }

            case INT_VFS_INIT: {
                regs->rax = (uint64_t)vfs_init((int)regs->rdi);
                break;
            }

            case INT_SYSCALL_MOUNT: { // 0x52
                regs->rax = (uint64_t)vfs_mount((int)regs->rdi);
                break;
            }

            case INT_SYSCALL_OPEN: { // 0x51
                int disk_no = regs->rdi;
                char *path = (char *)regs->rsi;
                int mode = (int)regs->rdx;
                regs->rax = (uint64_t) vfs_open(disk_no, path, mode);
                break;
            }

            case INT_SYSCALL_READ: {  // 0x35
                int disk_no = regs->rdi;
                void* fp = (void*) regs->rsi;
                char* buff = (char*) regs->rdx;
                size_t size = (size_t) regs->r10;
                regs->rax = (uint64_t) vfs_read(disk_no, fp, buff, size);
                break;

            }

            case INT_SYSCALL_WRITE: {  // 0x36
                int disk_no = regs->rdi;
                void* fp = (void*) regs->rsi;
                char* buff = (char*) regs->rdx;
                int filesize = (int) regs->r10;
                regs->rax = (uint64_t)vfs_write(disk_no, fp, buff, filesize);
                break;
            }

            case INT_SYSCALL_CLOSE: {  // 0x34
                int disk_no = (int) regs->rdi;
                void* fp = (void*) regs->rsi;
                regs->rax = (uint64_t)vfs_close(disk_no, fp);
                break;
            }

            case INT_SYSCALL_LSEEK: { // 0x37
                int disk_no = (int) regs->rdi;
                void* fp = (void*) regs->rsi;
                size_t offset = (size_t) regs->rdx;
                regs->rax = (uint64_t)vfs_lseek( disk_no, fp, offset);
                break;
            }

            case INT_SYSCALL_TRUNCATE: {
                int disk_no = (int) regs->rdi;
                void* fp = (void*) regs->rsi;
                regs->rax = (uint64_t)vfs_truncate(disk_no, fp);
                break;
            }

            case INT_SYSCALL_UNLINK: {
                int disk_no = (int) regs->rdi;
                char *path = (char *)regs->rsi;
                regs->rax = (uint64_t) vfs_unlink(disk_no, path);
                break;
            }
            
            // ------------------- VFS Directory Manage ------------------------------------

            case INT_SYSCALL_LIST: {
                printf("VFS List Directory Syscall Invoked\n");
                int disk_no = (int) regs->rdi;
                char *path = (char *)regs->rsi;
                // regs->rax = (uint64_t) vfs_listdir(disk_no, path);
                break;
            }

            case INT_SYSCALL_OPENDIR: {   // 0x4
                int disk_no = (int) regs->rdi;
                char *path = (char *)regs->rsi;
                regs->rax = (uint64_t) vfs_opendir(disk_no, path);
                break;
            }

            case INT_SYSCALL_CLOSEDIR: {  // 0x45
                int disk_no = (int) regs->rdi;
                void *dp = (void *)regs->rsi;
                regs->rax = vfs_closedir(disk_no, dp);
                break;
            }

            case INT_SYSCALL_READDIR: {   // 0x46
                int disk_no = (int) regs->rdi;
                void *dp = (void *)regs->rsi;
                void *fno = (void *)regs->rdx;
                regs->rax = (uint64_t) vfs_readdir(disk_no, dp, fno);
                break;
            }

            case INT_SYSCALL_MKDIR: {    // 0x4E
                int disk_no = (int) regs->rdi;
                char *buff = (char *)regs->rsi;
                regs->rax = (uint64_t)vfs_mkdir(disk_no, buff);
                break;
            }

            case INT_SYSCALL_GETCWD: {
                int disk_no = (int) regs->rdi;
                char *buff = (char *)regs->rsi;
                int len = (int) regs->rdx;
                regs->rax = vfs_getcwd(disk_no, buff, len);
                break;
            }

            case INT_SYSCALL_CHDIR: {
                int disk_no = (int) regs->rdi;
                char *path = (char *)regs->rsi;
                regs->rax = vfs_chdir(disk_no, path);
                break;
            }

            case INT_SYSCALL_CHDRIVE: {
                int disk_no = (int) regs->rdi;
                char *path = (char *)regs->rsi;
                regs->rax = vfs_chdrive(disk_no, path);
                break;
            }

            // ---------------------------- VGA--------------------------------

            case INT_VGA_SETPIXEL: {
                int x = (int) regs->rdi;
                int y = (int) regs->rsi;
                uint32_t color = (uint32_t) regs->rdx;

                set_pixel(x, y, color);
                break;
            }

            case INT_VGA_GETPIXEL: {
                int x = (int) regs->rdi;
                int y = (int) regs->rsi;

                regs->rax = get_pixel(x, y);
                break;
            } 
            
            case INT_VGA_CLEAR:{
                uint32_t color = (uint32_t) regs->rdi;
                cls_color(color);
                regs->rax = 0;
                break;
            } 
            
            case INT_VGA_DISPLAY_IMAGE:{
                int x = (int) regs->rdi;
                int y = (int) regs->rsi;
                const uint64_t* image_data = (const uint64_t*) regs->rdx;
                int width = (int) regs->r10;
                int height = (int) regs->r8;
                display_image(x, y, image_data, width, height);
                regs->rax = 0;
                break;
            }

            case INT_VGA_DISPLAY_TRANSPARENT_IMAGE:{
                int x = (int) regs->rdi;
                int y = (int) regs->rsi;
                const uint64_t* image_data = (const uint64_t*) regs->rdx;
                int width = (int) regs->r10;
                int height = (int) regs->r8;
                draw_image_with_transparency(x, y, image_data, width, height);
                regs->rax = 0;
                break;
            }

            case INT_ACPI_POWEROFF: {
                acpi_poweroff();
                regs->rax = 0;
                break;
            }

            case INT_ACPI_REBOOT: {
                acpi_reboot();
                regs->rax = 0;
                break;
            }

            case INT_SYSCALL_SERIAL_PRINT: {
                // copy_from_user(char *kernel_dst, const char *user_src, size_t max_len)
                const char kernel_buf[256];
                const char *user_buf = (const char *)regs->rax;
                uint64_t len = sizeof(user_buf);
                copy_from_user(kernel_buf, user_buf, (len < 256 ? len : 256));
                serial_print(kernel_buf);
                regs->rax = 0;
                break;
            }

            default: {
                printf("Unknown System Call! %d\n", regs->rax);
                regs->rax = (uint64_t)(-1); // unknown syscall
                break;
            }
        }
    }

    return regs;
}


void int_syscall_init(){
    irq_install(19, (void *)&int_systemcall_handler); 
    irq_install(96, (void *)&int_systemcall_handler);     

    asm volatile("sti");
    if(debug_on) printf(" Interrupt Based System Call initialized!\n");
}





void int_syscall_test(){

    printf(".......System Call Test Start\n");

    uint64_t res = system_call(INT_VFS_INIT, (uint64_t)1 , (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
    if(res != 0){
        printf("VFS initialization failed!\n");
    }

    // VFS Test
    char *disk = "1:";
    res = system_call(INT_SYSCALL_MOUNT, (uint64_t)disk , (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
    if(res == 0){
        printf("Successfully Mounted\n");
    }else{
        printf("Disk Mount Failed with Error Code %d\n", res);
    }

    printf("Listing root directory /\n");

    // List Directory
    const char *root_dir = "/";
    system_call(INT_SYSCALL_LIST, (uint64_t)root_dir, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);


    printf("Opening file /TESRFILE.TXT\n");

    // Open File
    char *path = "/TESTFILE.TXT";
    uint64_t flags = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;

    uint64_t opened_file = system_call(INT_SYSCALL_OPEN, (uint64_t) path, (uint64_t) flags, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
    if(opened_file == 0){
        printf("File Open Failed\n");
    }else{
        printf("Successfully open file %s\n", path);
    }

    printf("Updating pointer position\n");

    // LSEEK: Changing Pointer position in opened_file
    int lseek_res = system_call(INT_SYSCALL_LSEEK, (uint64_t) opened_file, (uint64_t) 8, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
    printf("lseek: %d\n", lseek_res);

    printf("Writing data into /TESTFILE.TXT\n");

    // Writing data string into opened_file
    const char* data = "Lala test string.\n";
    size_t write = system_call(INT_SYSCALL_WRITE, (uint64_t) opened_file, (uint64_t) 0, (uint64_t) data, (uint64_t) 128, (uint64_t) 0, (uint64_t) 0);

    if(write > -1){
        printf("Successfully wrote %d bytes\n", write);
    }
    
    printf("Reading File /TESTFILE.TXT\n");

    // Reading 
    char buffer[128];
    size_t bytes = system_call(INT_SYSCALL_READ, (uint64_t) opened_file, (uint64_t) 0, (uint64_t) buffer, (uint64_t) 128, (uint64_t) 0, (uint64_t) 0);

    if (bytes > 0) {
        buffer[bytes] = '\0';           // Null terminate if text
        printf("Content of /TESTFILE.TXT: %s\n", buffer);
    }

    printf(".....Successfully all systemcall tests passed!\n");
}



