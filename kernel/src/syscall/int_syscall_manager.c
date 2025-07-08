/*
Interrupt Based System Call

References: 
    https://github.com/dreamportdev/Osdev-Notes/blob/master/06_Userspace/04_System_Calls.md
*/

#include "../lib/string.h"  // for size_t
#include "../lib/stdio.h"

#include "../arch/interrupt/irq_manage.h"
#include "../util/util.h"
#include "../kshell/ring_buffer.h"
#include "../memory/kheap.h"
#include "../memory/uheap.h"
#include "../memory/paging.h"

#include "../FatFs-R0.15b/source/ff.h"        // FatFs library header
#include "../FatFs-R0.15b/source/diskio.h"    // FatFs

#include "../vfs/vfs.h"


#include "int_syscall_manager.h"

extern ring_buffer_t* keyboard_buffer;

extern FATFS  *fatfs;

void strcpy_from_user(char *dst, const char *user_src, size_t max_len) {
    size_t i = 0;
    while (i < max_len - 1) {
        char c = *((volatile char *)user_src + i);  // or use memory checking
        dst[i] = c;
        if (c == '\0') break;
        i++;
    }
    dst[i] = '\0'; // Ensure null termination
}

//  regs->rax is hold success(0) or error(-1) code
registers_t *int_systemcall_handler(registers_t *regs) {

    if(regs->int_no == 128){

        switch (regs->rax) { 

            case INT_SYSCALL_KEYBOARD_READ: {    // 0x59 : Read from keyboard buffer
                uint8_t *user_buf = (uint8_t *)regs->rdi;  // user buffer pointer
                
                if (!user_buf || regs->rsi == 0) {
                    regs->rax = (uint64_t)(-1); // error
                    break;
                }
                size_t size = regs->rsi;                   // max bytes to read
                size_t read_count = 0;

                while (read_count < size) {
                    uint8_t ch;
                    if (ring_buffer_pop(keyboard_buffer, &ch) == 0) {
                        user_buf[read_count++] = ch;
                    } else {
                        break;          // buffer empty
                    }
                }

                regs->rax = read_count; // Return number of bytes read
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

            /*
                "0:" → drive 0 (e.g. primary partition)

                "1:" → drive 1 (e.g. second disk or USB)

                "" → default drive (shortcut for "0:")

                Mount Option
                0	Delayed mount — mount is done automatically on first file access
                1	Immediate mount — mount the volume right now, return result immediately
            */

            case INT_SYSCALL_MOUNT: { // 0x52
                // Mount root FatFs volume to /
                char *path = (char *)regs->rdi; // Path to mount (unused in FatFs)
                BYTE opt = (BYTE)regs->rsi;     // Mount option
                FRESULT res = f_mount(fatfs, "", opt); // "" = default drive
                regs->rax = (res == FR_OK) ? 0 : -1;
                break;
            }

            case INT_SYSCALL_OPEN: { // 0x51
                char *user_path = (char *)regs->rdi;
                BYTE mode = (BYTE)regs->rsi;

                vfs_node_t* node = vfs_open(user_path, mode);

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
                DIR *dir = (DIR *)regs->rbx;
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
                DIR *dir = (DIR *)regs->rbx;
                if (!dir) {
                    regs->rax = (uint64_t)(-1);
                    break;
                }

                FILINFO fno;
                FRESULT res = f_readdir(dir, &fno);
                if (res != FR_OK) {
                    regs->rax = (uint64_t)(-1);
                } else {
                    // Return file name length and pointer to name
                    regs->rax = (uint64_t)fno.fname; // Pointer to file name
                    regs->rcx = fno.fname[0] ? strlen(fno.fname) : 0; // Length of file name
                }
                break;
            }

            case INT_SYSCALL_MKDIR: {    // 0x4E
                const char *path = (const char *)regs->rbx;
                if (!path) {
                    regs->rax = (uint64_t)(-1); // Invalid path
                    break;
                }

                FRESULT res = f_mkdir(path);
                regs->rax = (res == FR_OK) ? 0 : (uint64_t)(-1); // Return success or failure
                break;
            }

            default: {
                printf("Unknown System Call!\n");
                regs->rax = (uint64_t)(-1); // unknown syscall
                break;
            }
        }
    }

    return regs;
}



void int_syscall_init(){
    irq_install(128, (void *)&int_systemcall_handler);     

    printf(" [-] Interrupt Based System Call initialized!\n");
}

// rax, rdi, rsi, rdx, r10, r8, r9 

uint64_t system_call(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10, uint64_t r8, uint64_t r9){
    
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









