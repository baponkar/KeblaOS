
/*
    MSR System Call

    https://wiki.osdev.org/SYSENTER#AMD:_SYSCALL/SYSRET
*/
#include "../driver/vga/vga_term.h"
#include "../driver/io/ports.h"
#include "../lib/stdio.h"
#include "syscall_manager.h"

#define MSR_EFER     0xC0000080
#define MSR_STAR     0xC0000081
#define MSR_LSTAR    0xC0000082
#define MSR_SFMASK   0xC0000084

#define EFER_SCE  (1 << 0)    // Enable SYSCALL/SYSRET

#define USER_CS      0x1B     // User mode code selector (0x18 | 3)
#define KERNEL_CS    0x08     // Kernel mode code selector

extern void syscall_entry(); // from syscall_entry.asm


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



void syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2) {
    if(syscall_num == SYSCALL_PRINT){
        const char* str = (const char*)arg1;
        printf(str);  
    }else if(syscall_num == SYSCALL_READ){
        char* user_buf = (char*)arg1;
        uint64_t size = arg2;
    }else if(syscall_num == SYSCALL_EXIT){
        printf("User requested shell exit.\n");
        while (1) __asm__("hlt");
    }else{
        printf("Unknown syscall: %d\n", (int)syscall_num);
    }
}



void _syscall(uint64_t num, uint64_t arg1, uint64_t arg2) {
    __asm__ volatile (
        "syscall"
        :
        : "a"(num), "D"(arg1), "S"(arg2)
        : "rcx", "r11", "memory"
    );
}


