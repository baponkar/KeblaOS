

#include "../include/syscall.h"




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
        : [bytes] "=r" ((uint64_t)bytes_read)
        : [buf] "r" ((uint64_t)buffer), [size] "r" ((uint64_t)size)
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
        : [msg] "r" ((uint64_t)msg)
        : "rbx"
    );

    return 0; // success
}

int syscall_exit() {
    asm volatile ("int $174"); // Trigger exit syscall
    return 0; // This will not be reached, but for consistency
}

int syscall_print_rax() {
    asm volatile ("int $0x5C"); // Trigger print rax syscall
    return 0; // This will not be reached, but for consistency
}

uint64_t syscall_uheap_alloc(size_t size, enum allocation_type type) {
    if (size == 0) {
        return 0;
    }

    uint64_t ptr;

    asm volatile (
        "mov %[sz], %%rbx\n"      /* size → rbx */
        "mov %[tp], %%rcx\n"      /* type → rcx */
        "int $0x5D\n"             /* INT_SYSCALL_ALLOC */
        "mov %%rax, %[out]\n"     /* rax → ptr */
        : [out] "=r" (ptr)
        : [sz]  "r" ((uint64_t)size),
          [tp]  "r" ((uint64_t)type)
        : "rbx", "rcx", "rax"     /* only clobber what we actually use */
    );

    return ptr;
}


uint64_t uheap_free(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return -1; // Invalid pointer
    }

    uint64_t result;

    asm volatile (
        "mov %[ptr], %%rbx\n"
        "mov %[size], %%rcx\n"
        "int $0x5E\n" // Trigger uheap free syscall
        "mov %%rax, %[result]\n"
        : [result] "=r" (result)
        : [ptr] "r" ((uint64_t)ptr), [size] "r" ((uint64_t)size)
        : "rbx", "rax"
    );

    return result;
}

uint64_t syscall_fatfs_open(const char *path, uint64_t mode) {

    if (!path) {
        return -1; // Invalid path
    }

    uint64_t file_handle;

    asm volatile (
        "mov %[path], %%rbx\n"      // path pointer store into rbx
        "mov %[mode], %%rcx\n"      // Store mode into rdx
        "int $0x33\n"
        "mov %%rax, %[handle]\n"
        : [handle] "=r" (file_handle)
        : [path] "r" (path), [mode] "r" (mode)
        : "rbx", "rcx", "rax"
    );

    return file_handle;
}


uint64_t syscall_fatfs_close(void *file) {
    if (!file) {
        return -1; // Invalid file pointer
    }

    uint64_t result;

    asm volatile (
        "mov %[file], %%rbx\n"
        "int $0x34\n"
        "mov %%rax, %[result]\n"
        : [result] "=r" (result)
        : [file] "r" (file)
        : "rbx", "rax"
    );

    return result;
}

uint64_t syscall_fatfs_read(void *file, void *buf, uint32_t btr) {
    
    if (!file || !buf || btr == 0) {
        return -1; // Invalid parameters
    }

    uint64_t bytes_read;

    asm volatile (
        "mov %[file], %%rbx\n"      // Storing file pointer in rbx
        "mov %[buf], %%rcx\n"       // Storing buffer pointer in rcx
        "mov %[btr], %%rdx\n"
        "int $0x35\n"
        "mov %%rax, %[bytes]\n"
        : [bytes] "=r" (bytes_read)
        : [file] "r" ((uint64_t)file), [buf] "r" (buf), [btr] "r" ((uint64_t)btr)
        : "rbx", "rcx", "rdx", "rax"
    );

    return bytes_read;
}

uint64_t syscall_fatfs_write(void *file, const void *buf, uint32_t btw) {
    if (!file || !buf || btw == 0) {
        return -1; // Invalid parameters
    }

    uint64_t bytes_written;

    asm volatile (
        "mov %[file], %%rbx\n"
        "mov %[buf], %%rcx\n"
        "mov %[btw], %%rdx\n"
        "int $0x36\n"
        "mov %%rax, %[bytes]\n"
        : [bytes] "=r" (bytes_written)
        : [file] "r" ((uint64_t)file), [buf] "r" (buf), [btw] "r" ((uint64_t)btw)
        : "rbx", "rcx", "rdx", "rax"
    );

    return bytes_written;
}


 /*
    "0:" → drive 0 (e.g. primary partition)

    "1:" → drive 1 (e.g. second disk or USB)

    "" → default drive (shortcut for "0:")

    0	Delayed mount — mount is done automatically on first file access
    1	Immediate mount — mount the volume right now, return result immediately
*/

uint64_t syscall_fatfs_mount(char *path, uint8_t opt) {
    uint64_t result;

    asm volatile (
        "mov %[path], %%rbx\n"
        "mov %[opt], %%rcx\n"
        "int $0x52\n" // Trigger mount syscall
        "mov %%rax, %[result]\n"
        : [result] "=r" (result)
        : [path] "r" (path), [opt] "r" ((uint64_t)opt)
        : "rbx", "rcx", "rax"
    );

    return result;
}