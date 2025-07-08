
#include "../include/stdio.h"

#include "../include/syscall.h"

// Userside system call function to manage all system call
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



int syscall_keyboard_read(uint8_t *buffer, size_t size) {
    if (!buffer || size == 0) {
        return -1; // Invalid parameters
    }

    return system_call((uint64_t) INT_SYSCALL_KEYBOARD_READ, (uint64_t) buffer, (uint64_t) size, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}


int syscall_print(const char *msg) {
    if (!msg) {
        return -1; // Invalid message
    }

    return system_call((uint64_t) INT_SYSCALL_PRINT, (uint64_t) msg, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

int syscall_exit() {
    return system_call((uint64_t) INT_SYSCALL_EXIT, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);    
}

int syscall_print_rax() {

    return system_call((uint64_t) INT_SYSCALL_PRINT_RAX, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}


uint64_t syscall_uheap_alloc(size_t size, enum allocation_type type) {
    if (size == 0) {
        return 0;
    }

    return system_call((uint64_t) INT_SYSCALL_ALLOC, (uint64_t) size, (uint64_t) type, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}


uint64_t uheap_free(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return -1; // Invalid pointer
    }

    return system_call((uint64_t) INT_SYSCALL_FREE, (uint64_t) ptr, (uint64_t) size, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}

// FatFs functions used

 /*
    "0:" → drive 0 (e.g. primary partition)

    "1:" → drive 1 (e.g. second disk or USB)

    "" → default drive (shortcut for "0:")

    0	Delayed mount — mount is done automatically on first file access
    1	Immediate mount — mount the volume right now, return result immediately
*/


uint64_t syscall_mount(char *path, uint8_t opt) {
    return system_call((uint64_t) INT_SYSCALL_FATFS_MOUNT, (uint64_t) path, (uint64_t) opt, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}


uint64_t syscall_open(const char *path, uint64_t mode) {

    if (!path) {
        return -1;                  // Invalid path
    }

    return system_call((uint64_t) INT_SYSCALL_FATFS_OPEN, (uint64_t) path, (uint64_t) mode, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
    
}


uint64_t syscall_close(void *file) {
    if (!file) {
        return -1; // Invalid file pointer
    }

    return system_call((uint64_t) INT_SYSCALL_FATFS_CLOSE, (uint64_t) file, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_read(void *file, void *buf, uint32_t btr) {
    
    if (!file || !buf || btr == 0) {
        return -1; // Invalid parameters
    }

    return system_call((uint64_t) INT_SYSCALL_FATFS_READ, (uint64_t) buf, (uint64_t) btr, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_write(void *file, void *buf, uint32_t btw) {
    
    if (!file || !buf || btw == 0) {
        return (uint64_t)-1;    // Invalid parameters
    }

    return system_call((uint64_t) INT_SYSCALL_FATFS_WRITE, (uint64_t) file, (uint64_t) buf, (uint64_t) btw, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}


uint64_t syscall_lseek(void *file, uint32_t offs) {
    if (!file) {
        return -1;                  // Invalid parameters
    }

    return system_call((uint64_t) INT_SYSCALL_FATFS_LSEEK, (uint64_t) file, (uint64_t) offs, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}
