
/*
MSR System Call
*/

#include "../driver/io/ports.h"
#include "../lib/stdio.h"
#include "syscall.h"

#define MSR_EFER     0xC0000080
#define MSR_STAR     0xC0000081
#define MSR_LSTAR    0xC0000082
#define MSR_SFMASK   0xC0000084

#define EFER_SCE  (1 << 0)  // Enable SYSCALL/SYSRET

#define USER_CS      0x1B  // User mode code selector (0x18 | 3)
#define KERNEL_CS    0x08  // Kernel mode code selector

static inline uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static inline void write_msr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile ("wrmsr" :: "c"(msr), "a"(low), "d"(high));
}

extern void syscall_entry(); // from syscall_entry.asm

void init_syscall() {
    // Enable SYSCALL/SYSRET by setting SCE in IA32_EFER.
    uint64_t efer = read_msr(MSR_EFER);
    efer |= EFER_SCE;
    write_msr(MSR_EFER, efer);

    // STAR: sets up CS/SS for kernel (bits 32-47) and user (bits 48-63)
    uint64_t star = ((uint64_t)USER_CS << 48) | ((uint64_t)KERNEL_CS << 32);
    write_msr(MSR_STAR, star);

    // LSTAR: address of our syscall entry point.
    write_msr(MSR_LSTAR, (uint64_t)&syscall_entry);

    // SFMASK: mask IF (and maybe other flags) when transitioning.
    write_msr(MSR_SFMASK, 1 << 9);
}

void syscall_handler(uint64_t syscall_num, uint64_t arg1) {
    if (syscall_num == 1) {
        printf("User called syscall 1 with arg %d\n", (int)arg1);
    }
}


