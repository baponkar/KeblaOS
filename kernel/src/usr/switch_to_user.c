/*
Switch into User Mode

References:
    https://github.com/dreamportdev/Osdev-Notes/blob/master/06_Userspace/02_Switching_Modes.md
    https://wiki.osdev.org/Getting_to_Ring_3
    https://hasinisama.medium.com/operating-system-user-mode-2c2f19ca8e43
    https://hasinisama.medium.com/operating-systems-the-road-to-user-mode-854b72221b1b
*/


#include "../memory/paging.h"
#include "../memory/uheap.h"
#include "../memory/vmm.h"

#include "../util/util.h"
#include  "../lib/stdio.h"
#include  "../lib/string.h"

#include "switch_to_user.h"


#define KERNEL_CS 0x08  // 0x08 | 0
#define KERNEL_SS 0x10  // 0x10 | 0
#define USER_CS 0x1B    // 0x18 | 3
#define USER_SS 0x23    // 0x20 | 3 = 100000 | 11 = 100011 = 0x23

#define STACK_SIZE 0x4000   // 16 kb

extern void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr); // Defined in switch_user.asm

int is_user_mode() {
    uint64_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    return (cs & 0x3) == 3;     // If CPL (bits 0-1) == 3, then it's user mode
}


uint64_t create_user_function() {
    char *hello_str = (char *)uheap_alloc(24, ALLOCATE_DATA); // "Hello from User Program\n" + '\0'
    strcpy(hello_str, "Hello from User Program\n");
    uint64_t str_addr = (uint64_t)hello_str;

    void *user_code = (void *) uheap_alloc(0x1000, ALLOCATE_CODE); // Allocate 4kb for user code

    // The machine code determined from user_programe.asm
    // nasm -f bin module/user_programe.asm -o module/user_programe.bin
    // objdump -D -b binary -m i386:x86-64 module/user_programe.bin

    uint8_t user_program[] = {
        // mov rbx, <str_addr>
        0x48, 0xbb,
        (str_addr >> 0) & 0xff,
        (str_addr >> 8) & 0xff,
        (str_addr >> 16) & 0xff,
        (str_addr >> 24) & 0xff,
        (str_addr >> 32) & 0xff,
        (str_addr >> 40) & 0xff,
        (str_addr >> 48) & 0xff,
        (str_addr >> 56) & 0xff,

        // mov rax, 0xad
        // 0xb8, 0xad, 0x00, 0x00, 0x00,

        // int 0xad
        0xcd, 0xad,

        // jmp $
        0xeb, 0xfe
    };

    memcpy(user_code, (void*)&user_program, sizeof(user_program));

    return (uint64_t)user_code; 
}


void init_user_mode(){
    
    uint64_t code_addr = (uint64_t) create_user_function();
    uint64_t stack_top_addr = ((uint64_t) uheap_alloc(STACK_SIZE, ALLOCATE_STACK)) + STACK_SIZE; // Allocate 16kb for stack and align to page boundary

    printf("Starting Switching to the user mode: code addr.- %x, stack top addr.- %x\n", code_addr, stack_top_addr);

    switch_to_user_mode(stack_top_addr, code_addr);
}


