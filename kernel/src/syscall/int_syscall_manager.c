/*
Interrupt Based System Call

References: 
    https://github.com/dreamportdev/Osdev-Notes/blob/master/06_Userspace/04_System_Calls.md
*/

#include "../lib/string.h"  // for size_t
#include "../lib/stdio.h"

#include "../process/process.h"
#include "../process/thread.h"

#include "../arch/interrupt/irq_manage.h"
#include "../util/util.h"
#include "../driver/keyboard/ring_buffer.h"
#include "../memory/kheap.h"
#include "../memory/uheap.h"
#include "../memory/paging.h"

#include "../fs/FatFs-R0.15b/source/ff.h"        // FatFs library header
#include "../fs/FatFs-R0.15b/source/diskio.h"    // FatFs

#include "../vfs/vfs.h"

#include "../sys/timer/time.h"


#include "int_syscall_manager.h"

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
  kernel_dst[max_len - 1] = '\0'; // Ensure termination
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

            // --------------------------FATFS File Manages--------------------------

            case INT_SYSCALL_MOUNT: { // 0x52
                // Mount root FatFs volume to /
                char *path = (char *)regs->rdi; // Path to mount (unused in FatFs)
                BYTE opt = (BYTE)regs->rsi;     // Mount option
                FRESULT res = f_mount(fatfs, "", opt); // "" = default drive
                regs->rax = (res == FR_OK) ? 0 : -1;
                break;
            }

            case INT_SYSCALL_OPEN: { // 0x51
                char *path = (char *)regs->rdi;
                BYTE mode = (BYTE)regs->rsi;

                // Copy path to kernel-space buffer
                char kernel_path[MAX_PATH_LEN]; // MAX_PATH_LEN = 256
                if (copy_from_user(kernel_path, path, MAX_PATH_LEN) != 0) {
                    regs->rax = -1;             // Copy failed
                    break;
                }

                vfs_node_t* node = vfs_open(kernel_path, mode);

                regs->rax = (node != NULL) ? (uint64_t) node : -1;
                break;
            }

            case INT_SYSCALL_READ: {  // 0x35
                vfs_node_t *node = (vfs_node_t *)regs->rdi;
                void *buf = (void *)regs->rsi;
                uint64_t size = regs->rdx;

                uint64_t out = vfs_read(node, buf, size);

                regs->rax = (out > 0) ? out : -1;

                break;

            }

            case INT_SYSCALL_WRITE: {  // 0x36
                vfs_node_t *node = (vfs_node_t *)regs->rdi;
                void *buf = (const void *)regs->rsi;
                uint64_t size = regs->rdx;

                uint64_t out = vfs_write(node, buf, size);

                regs->rax = (out > 0) ? out : -1;
                
                break;
            }

            case INT_SYSCALL_CLOSE: {  // 0x34
                vfs_node_t *node = (vfs_node_t *)regs->rdi;

                if(!node){
                    regs->rax = (-1);
                }

                regs->rax = vfs_close(node);
                break;
            }

            case INT_SYSCALL_LSEEK: { // 0x37
                vfs_node_t *node = (vfs_node_t *)regs->rdi;
                FSIZE_t offset = (FSIZE_t) regs->rsi;

                if(!node){
                    regs->rax = (uint64_t)(-1);
                    break;
                }
        
                FRESULT res =  vfs_lseek( node, offset);
                regs->rax = (res == FR_OK) ? 0 : -1;
                break;
            }

            case INT_SYSCALL_TRUNCATE: {
                char *path = (char *)regs->rdi;
                uint64_t offset = (uint64_t) regs->rsi;
                
                if(!path){
                    regs->rax = (uint64_t)(-1);
                    break;
                }

                // Copy path to kernel-space buffer
                char kernel_path[MAX_PATH_LEN]; // e.g., MAX_PATH_LEN = 256
                if (copy_from_user(kernel_path, path, MAX_PATH_LEN) != 0) {
                    regs->rax = -1; // Copy failed
                    break;
                }

                vfs_node_t *node = vfs_open(kernel_path, FA_OPEN_EXISTING);
                
                FRESULT res = vfs_truncate(node, offset);
                regs->rax = (res == FR_OK) ? 0 : -1;
            }

            case INT_SYSCALL_UNLINK: {
                char *path = (char *)regs->rdi;

                if(!path) {
                    regs->rax = (uint64_t)-1;
                    break;
                }

                // Copy path to kernel-space buffer
                char kernel_path[MAX_PATH_LEN]; // e.g., MAX_PATH_LEN = 256
                if (copy_from_user(kernel_path, path, MAX_PATH_LEN) != 0) {
                    regs->rax = -1; // Copy failed
                    break;
                }

                regs->rax = vfs_unlink(kernel_path);
                break;
            }
            
            // ------------------- FATFS Directory Manage ------------------------------------

            case INT_SYSCALL_OPENDIR: {   // 0x44
                const char *path = (const char *)regs->rdi;
                if (!path) {
                    regs->rax = (uint64_t)(-1); // Invalid path
                    break;
                }

                DIR *dir = kheap_alloc(sizeof(DIR), ALLOCATE_DATA);
                if (!dir) {
                    kheap_free(dir, sizeof(DIR));
                    regs->rax = (uint64_t)(-1); // Memory allocation failed
                    break;
                }
                FRESULT res = f_opendir(dir, path);
                if (res != FR_OK) {
                    kheap_free(dir, sizeof(DIR));
                    regs->rax = (uint64_t)(-1); // Open directory failed
                } else {
                    regs->rax = (uint64_t)dir; // Return directory pointer
                }
                break;
            }

            case INT_SYSCALL_CLOSEDIR: {  // 0x45
                DIR *dir = (DIR *)regs->rdi;
                if (!dir) {
                    regs->rax = (uint64_t)(-1);
                    break;
                }

                FRESULT res = f_closedir(dir);
                kheap_free(dir, sizeof(DIR));
                regs->rax = (res == FR_OK) ? 0 : (uint64_t)(-1);
                break;
            }

            case INT_SYSCALL_READDIR: {   // 0x46
                DIR *dir = (DIR *)regs->rdi;
                static char static_fname[256];  // Static buffer
                
                if (!dir) {
                    regs->rax = (uint64_t)(-1);
                    break;
                }

                FILINFO fno;
                memset(&fno, 0, sizeof(FILINFO));
                FRESULT res = f_readdir(dir, &fno);
                if (res != FR_OK || fno.fname[0] == 0) {
                    regs->rax = 0; // No more entries
                } else {
                    strcpy(static_fname, fno.fname);
                    regs->rax = (uint64_t)static_fname;
                    regs->rcx = strlen(static_fname);
                }
                break;
            }

            case INT_SYSCALL_MKDIR: {    // 0x4E
                const char *path = (const char *)regs->rdi;
                if (!path) {
                    regs->rax = (uint64_t)(-1); // Invalid path
                    break;
                }

                FRESULT res = f_mkdir(path);
                regs->rax = (res == FR_OK) ? 0 : (uint64_t)(-1); // Return success or failure
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
    printf(" [-] Interrupt Based System Call initialized!\n");
}







