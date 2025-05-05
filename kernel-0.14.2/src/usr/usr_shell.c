
#include "../lib/stdio.h"
#include "../lib/string.h"

#include "usr_shell.h"

#define SYS_READ  0
#define SYS_PRINT 1

#define BUFFER_SIZE 128

// Simulate system call (assuming int 0xAC invokes kernel syscall dispatcher)
static inline long syscall(int syscall_number, void *buffer, size_t size) {
    long ret;
    asm volatile (
        "mov %[num], %%rax\n"
        "mov %[buf], %%rbx\n"
        "mov %[siz], %%rcx\n"
        "int $0xAC\n"
        "mov %%rax, %[ret]"
        : [ret] "=r"(ret)
        : [num] "r"((uint64_t)syscall_number),
          [buf] "r"(buffer),
          [siz] "r"(size)
        : "rax", "rbx", "rcx"
    );
    return ret;
}

void ushell() {
    char *buffer = (char *)0x400000;  // Assuming you allocate this via uheap_alloc
    syscall(SYS_PRINT, "Welcome to ushell (User Mode Shell)\n", 0);

    while (1) {
        syscall(SYS_PRINT, "UserShell>> ", 0);

        // Read input
        syscall(SYS_READ, buffer, BUFFER_SIZE);

        // Compare input with known commands
        if (buffer[0] == '\0') continue;
        if (strcmp(buffer, "help") == 0) {
            syscall(SYS_PRINT, "help, echo, exit\n", 0);
        } else if (strcmp(buffer, "exit") == 0) {
            syscall(SYS_PRINT, "Exiting ushell...\n", 0);
            break;
        } else if (strncmp(buffer, "echo ", 5) == 0) {
            syscall(SYS_PRINT, buffer + 5, 0);
            syscall(SYS_PRINT, "\n", 0);
        } else {
            syscall(SYS_PRINT, "Unknown command\n", 0);
        }
    }

    while (1) { asm volatile ("hlt"); }
}
