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
    void *user_code = (void *) uheap_alloc(0x1000);

    // The machine code determined from user_programe.asm
    // nasm -f bin module/user_programe.asm -o module/user_programe.bin
    // objdump -D -b binary -m i386:x86-64 module/user_programe.bin
    uint8_t user_program[] = {
        // 0x31, 0xc9,                     // xor    %ecx,%ecx
        // 0xf7, 0xf1,                     // div    %ecx
        0xb8, 0x78, 0x56, 0x34, 0x12,   // mov $0x12345678,%eax
        0xcd, 0x1,                     // int $0xac,
        0xeb, 0xfe                      // jmp 0x9
    };

    memcpy(user_code, (void*)&user_program, sizeof(user_program));

    return (uint64_t)user_code; // Return the user-accessible function pointer
}

uint64_t create_user_function_1() {
    void *user_code = (void *) uheap_alloc(0x1000);  // must be mapped with user-mode permissions

    // Machine code:
    // mov rax, 1                ; syscall number (SYSCALL_PRINT)
    // mov rdi, addr_of_string   ; arg1 (pointer to string)
    // syscall
    // jmp $

    const char *message = "Hello from user syscall!\n";

    uint64_t str_addr = (uint64_t)user_code + 0x100;  // place string after code
    memcpy((void*)str_addr, message, strlen(message) + 1);

    uint8_t user_program[] = {
        0x48, 0xB8,              // mov rax, imm64
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // SYSCALL_PRINT = 1

        0x48, 0xBF,              // mov rdi, imm64
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // address of string

        0x0F, 0x05,              // syscall

        0xEB, 0xFE               // jmp $
    };

    // Fill in the syscall number
    *(uint64_t *)(user_program + 2) = 1;  // SYSCALL_PRINT

    // Fill in the string address
    *(uint64_t *)(user_program + 12) = str_addr;

    // Copy program to memory
    memcpy(user_code, user_program, sizeof(user_program));

    return (uint64_t)user_code;
}


void init_user_mode(){
    
    uint64_t code_addr = (uint64_t) create_user_function();
    printf("User function code address: %x\n", code_addr);
    page_t *code_page = get_page(code_addr, 0, (pml4_t *)get_cr3_addr());
    code_page->rw = 0;          // Making readable only
    code_page->nx = 0;          // Making executable
    code_page->user = 1;        // Making User accessible

    uint64_t stack_base_addr = ((uint64_t) uheap_alloc(STACK_SIZE));
    for(uint64_t addr = stack_base_addr; addr < stack_base_addr + STACK_SIZE; addr += 0x1000){
        page_t *stack_page = get_page(addr, 0,  (pml4_t *)get_cr3_addr());
        stack_page->rw = 1;     // Making it read-writable
        stack_page->nx = 1;     // Making non-executable
        stack_page->user = 1;   // Making User accessible
    }
    uint64_t stack_top_addr = stack_base_addr + STACK_SIZE;   // Set stack top
    
    flush_tlb_all();
    
    printf("Starting Switching to the user mode: code addr.- %x, stack addr.- %x\n", code_addr, stack_top_addr);
    switch_to_user_mode(stack_top_addr, code_addr);
}


