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

#include "int_syscall_manager.h"

extern ring_buffer_t* keyboard_buffer;

static FATFS  fatfs;

//  regs->rax is hold success(0) or error(-1) code
registers_t *int_systemcall_handler(registers_t *regs) {
    switch (regs->int_no) {                             
        case INT_SYSCALL_READ: {    // 0x59 : Read from keyboard buffer
            uint8_t *user_buf = (uint8_t *)regs->rbx;  // user buffer pointer
            
            if (!user_buf || regs->rcx == 0) {
                regs->rax = -1; // error
                break;
            }
            size_t size = regs->rcx;                   // max bytes to read
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
            if (!regs->rbx) {
                printf("Invalid string pointer!\n");
                regs->rax = -1;
                break;
            }

            const char *str = (const char *)regs->rbx;
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

        case INT_SYSCALL_FATFS_OPEN: {  // 0x33 : Open a file

            const char *path = (const char *)regs->rbx;
            if (!path) {
                regs->rax = -1; // Invalid path
                break;
            }

            // FIL *file = (FIL *)regs->rcx; // File pointer (output)
            FIL *file = (FIL *)kheap_alloc(sizeof(FIL), ALLOCATE_DATA);
            if (!file) {
                regs->rax = -1; // Memory allocation failed
                break;
            }

            BYTE mode = (BYTE)regs->rcx; // File open mode
            FRESULT res = f_open(file, path, mode);
            if (res != FR_OK) {
                kheap_free((void *)file, sizeof(FIL));
                regs->rax = -1; // Open file failed
            } else {
                regs->rax = (uint64_t)file; // Return file pointer
            }
            break;
        }

        case INT_SYSCALL_FATFS_CLOSE: { // 0x34 : Close a file
            FIL *file = (FIL *)regs->rbx;
            if (!file) {
                regs->rax = -1;
                break;
            }

            FRESULT res = f_close(file);
            
            kheap_free((void *)file, sizeof(FIL));
            regs->rax = (res == FR_OK) ? 0 : -1;
            break;
        }

        case INT_SYSCALL_FATFS_READ: {  // 0x35 : Read from a file
            FIL *file = (FIL *)regs->rbx;
            void *buf = (void *)regs->rcx;
            UINT btr = (UINT)regs->rdx; // Bytes to read(input)
            UINT br; // Bytes read(output)

            if (!file || !buf || btr == 0) {
                regs->rax = -1;
                break;
            }

            FRESULT res = f_read(file, buf, btr, &br);
            regs->rax = (res == FR_OK) ? br : -1;
            break;
        }

        case INT_SYSCALL_FATFS_WRITE: { // 0x36 : Write to a file
            FIL *file = (FIL *)regs->rbx;
            const void *buf = (const void *)regs->rcx;
            UINT btw = (UINT)regs->rdx; // Bytes to write(input)
            UINT bw;    // Bytes written(output)

            if (!file || !buf || btw == 0) {
                regs->rax = -1; // Invalid parameters
                break;
            }

            FRESULT res = f_write(file, buf, btw, &bw);
            regs->rax = (res == FR_OK) ? bw : -1; // Return number of bytes written
            break;
        }

        /*
            "0:" → drive 0 (e.g. primary partition)

            "1:" → drive 1 (e.g. second disk or USB)

            "" → default drive (shortcut for "0:")

            0	Delayed mount — mount is done automatically on first file access
            1	Immediate mount — mount the volume right now, return result immediately
        */
        case INT_SYSCALL_FATFS_MOUNT: { // 0x52

            char *path = (char *)regs->rbx; // Path to mount
            if (!path) {
                regs->rax = -1; // Invalid path
                break;
            }

            BYTE opt = (BYTE)regs->rcx; // Mount options
            if (opt > 0xFF) {
                printf("Invalid mount options!\n");
                regs->rax = -1; // Invalid options
                break;
            }

            FRESULT res = f_mount((FATFS *)&fatfs, path, opt);
            if (res != FR_OK) {
                regs->rax = -1;
            } else {
                regs->rax = (uint64_t)&fatfs;
            }
            break;
        }

        case INT_SYSCALL_FATFS_OPENDIR: {   // 0x44
            const char *path = (const char *)regs->rbx;
            if (!path) {
                regs->rax = -1; // Invalid path
                break;
            }

            DIR *dir = kheap_alloc(sizeof(DIR), ALLOCATE_DATA);
            if (!dir) {
                kheap_free(dir, sizeof(DIR));
                regs->rax = -1; // Memory allocation failed
                break;
            }
            FRESULT res = f_opendir(dir, path);
            if (res != FR_OK) {
                kheap_free(dir, sizeof(DIR));
                regs->rax = -1; // Open directory failed
            } else {
                regs->rax = (uint64_t)dir; // Return directory pointer
            }
            break;
        }

        case INT_SYSCALL_FATFS_CLOSEDIR: {  // 0x45
            DIR *dir = (DIR *)regs->rbx;
            if (!dir) {
                regs->rax = -1;
                break;
            }

            FRESULT res = f_closedir(dir);
            kheap_free(dir, sizeof(DIR));
            regs->rax = (res == FR_OK) ? 0 : -1;
            break;
        }

        case INT_SYSCALL_FATFS_READDIR: {   // 0x46
            DIR *dir = (DIR *)regs->rbx;
            if (!dir) {
                regs->rax = -1;
                break;
            }

            FILINFO fno;
            FRESULT res = f_readdir(dir, &fno);
            if (res != FR_OK) {
                regs->rax = -1;
            } else {
                // Return file name length and pointer to name
                regs->rax = (uint64_t)fno.fname; // Pointer to file name
                regs->rcx = fno.fname[0] ? strlen(fno.fname) : 0; // Length of file name
            }
            break;
        }

        case INT_SYSCALL_FATFS_MKDIR: {    // 0x4E
            const char *path = (const char *)regs->rbx;
            if (!path) {
                regs->rax = -1; // Invalid path
                break;
            }

            FRESULT res = f_mkdir(path);
            regs->rax = (res == FR_OK) ? 0 : -1; // Return success or failure
            break;
        }

        case INT_SYSCALL_ALLOC: { // 0x5D : Allocate memory
            size_t size = regs->rbx;
            uint8_t type = regs->rcx;

            if (size == 0 || type > 0x3) {
                printf("Invalid type parameters for allocation syscall!\n");
                regs->rax = -1; // error
                break;
            }
            uint64_t ptr = (uint64_t) uheap_alloc(size, type);

            if(!ptr) {
                printf("Memory allocation failed!\n");
                regs->rax = -1; // error
                break;
            }
            regs->rax = (uint64_t)ptr;
            break;
        }

        case INT_SYSCALL_FREE: {    // 0x5E : Free allocated memory
            void *ptr = (void *)regs->rbx;
            if (!ptr) {
                printf("Invalid pointer for free syscall!\n");
                regs->rax = -1; // error
                break;
            }
            size_t size = regs->rcx;
            if (size == 0) {
                printf("Invalid size for free syscall!\n");
                regs->rax = -1; // error
                break;
            }
            uheap_free(ptr, size);
            regs->rax = 0; // success
            break;
        }

        default: {
            printf("Unknown System Call!\n");
            regs->rax = -1; // unknown syscall
            break;
        }
    }

    return regs;
}



void int_syscall_init(){
    for(int i = 19; i <= 62; i++) {
        irq_install(i, (void *)&int_systemcall_handler); 
    }

    printf(" [-] Interrupt Based System Call initialized!\n");
}


int syscall_read(uint8_t *buffer, size_t size) {
    if (!buffer || size == 0) {
        return -1; // Invalid parameters
    }

    size_t bytes_read = 0;

    asm volatile (
        "mov %[buf], %%rbx\n"
        "mov %[size], %%rcx\n"
        "int $172\n" // Trigger read syscall
        "mov %%rax, %[bytes]\n"
        : [bytes] "=r" (bytes_read)
        : [buf] "r" (buffer), [size] "r" (size)
        : "rbx", "rcx", "rax"
    );

    return bytes_read;
}


int syscall_print(const char *msg) {
    if (!msg) {
        return -1; // Invalid message
    }

    asm volatile (
        "mov %[msg], %%rbx\n"
        "int $0x5A\n" // Trigger print syscall
        :
        : [msg] "r" (msg)
        : "rbx"
    );

    return 0; // success
}

int syscall_exit() {
    asm volatile ("int $174"); // Trigger exit syscall
    return 0; // This will not be reached, but for consistency
}


void syscall_test(){
    printf("Testing Interrupt Based System Call...\n");

    // Test read syscall
    uint8_t buffer[64];
    int bytes_read = syscall_read(buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the string
        printf("Read %d bytes: %s\n", bytes_read, buffer);
    } else {
        printf("Read syscall failed or no input available.\n");
    }

    // Test print syscall
    const char *test_str = "Hello from Interrupt Based System Call!";
    syscall_print(test_str);

    // Test exit syscall
    syscall_exit();

    // Creating a file
}




