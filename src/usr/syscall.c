/*
System Call

https://github.com/dreamportdev/Osdev-Notes/blob/master/06_Userspace/04_System_Calls.md

*/

#include "../lib/stdio.h"

#include "syscall.h"

// MSR
#define MSR_IA32_STAR    0xC0000081
#define MSR_IA32_LSTAR   0xC0000082
#define MSR_IA32_CSTAR   0xC0000083
#define MSR_IA32_SFMASK  0xC0000084


// Define your segment selectors.
#define KERNEL_CS 0x08       // Kernel code segment (Ring 0)
#define USER_CS   0x33       // User code segment (Ring 3)
// Note: The values for IA32_STAR are constructed as follows:
//    IA32_STAR[47:32] = kernel CS and data segment selector (KERNEL_CS and KERNEL_DS)
//    IA32_STAR[63:48] = user code segment selector (USER_CS)

// Set the system call entry point (LSTAR) to our syscall_handler.
extern void syscall_entry();  // This symbol should be defined in your assembly syscall handler.

static inline void wrmsr(uint32_t msr, uint64_t value) {
    __asm__ volatile("wrmsr" : : "c"(msr), "a"((uint32_t)value), "d"((uint32_t)(value >> 32)));
}

void init_syscall(void) {
    uint64_t star_value = ((uint64_t)USER_CS << 48) | ((uint64_t)KERNEL_CS << 32);
    wrmsr(MSR_IA32_STAR, star_value);

    wrmsr(MSR_IA32_LSTAR, (uint64_t) &syscall_entry);

    // Set the flag mask to disable interrupts in syscall handler if desired.
    // For example, mask out the IF flag (bit 9):
    wrmsr(MSR_IA32_SFMASK, 1 << 9);
}




// Prototype for the syscall dispatcher
void syscall_dispatcher(void) {
    // The syscall number is typically in RAX.
    uint64_t syscall_num;
    __asm__ volatile ("mov %%rax, %0" : "=r"(syscall_num));
    
    // Dispatch the syscall based on its number.
    switch (syscall_num) {
        case 0:
            // For example, a simple "hello" syscall.
            printf("Hello from syscall 0!\n");
            break;
        // Add additional cases for other syscalls.
        default:
            printf("Unknown syscall: %lu\n", syscall_num);
            break;
    }
    
    // Optionally set a return value in RAX.
    __asm__ volatile ("mov $0, %%rax" : : : "rax");
}





static inline uint64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2,
                                 uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    register uint64_t rax __asm__("rax") = num;
    register uint64_t rdi __asm__("rdi") = arg1;
    register uint64_t rsi __asm__("rsi") = arg2;
    register uint64_t rdx __asm__("rdx") = arg3;
    register uint64_t r10 __asm__("r10") = arg4;
    register uint64_t r8  __asm__("r8")  = arg5;
    register uint64_t r9  __asm__("r9")  = arg6;
    __asm__ volatile (
        "syscall"
        : "+r" (rax)
        : "r" (rdi), "r" (rsi), "r" (rdx),
          "r" (r10), "r" (r8), "r" (r9)
        : "rcx", "r11", "memory"
    );
    return rax;
}

void uses_syscall() {
    // For example, make syscall 0.
    uint64_t ret = syscall(0, 0, 0, 0, 0, 0, 0);
}

