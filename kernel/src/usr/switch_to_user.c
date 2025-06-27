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


#define KERNEL_CS 0x08      // 0x08 | 0
#define KERNEL_SS 0x10      // 0x10 | 0
#define USER_CS 0x1B        // 0x18 | 3
#define USER_SS 0x23        // 0x20 | 3 = 100000 | 11 = 100011 = 0x23

#define STACK_SIZE 0x4000   // 16 kb

extern void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr); // Defined in switch_user.asm

int is_user_mode() {
    uint64_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    return (cs & 0x3) == 3;     // If CPL (bits 0-1) == 3, then it's user mode
}


uint64_t create_user_function() {

    void *user_code = (void *) uheap_alloc(0x1000, ALLOCATE_CODE);  // 4KB user code
    char *prompt_str = (char *)uheap_alloc(32, ALLOCATE_DATA);      // prompt string
    char *read_buf   = (char *)uheap_alloc(64, ALLOCATE_DATA);      // buffer for user input

    strcpy(prompt_str, "Enter input: ");
    uint64_t str_addr = (uint64_t)prompt_str;
    uint64_t read_buf_addr = (uint64_t)read_buf;

    char *filename = (char *)uheap_alloc(32, ALLOCATE_DATA);
    strcpy(filename, "test.txt");
    uint64_t filename_ptr = (uint64_t)filename;


    uint8_t user_program[] = {
        // --- PRINT "Enter input: " ---
        0x48, 0xBB,                            // mov rbx, <str_addr>
        (str_addr >> 0) & 0xFF,
        (str_addr >> 8) & 0xFF,
        (str_addr >> 16) & 0xFF,
        (str_addr >> 24) & 0xFF,
        (str_addr >> 32) & 0xFF,
        (str_addr >> 40) & 0xFF,
        (str_addr >> 48) & 0xFF,
        (str_addr >> 56) & 0xFF,

        0xCD, 0x5A,                                 // int 0x5A (print syscall)

        // --- READ input into buffer ---
        0x48, 0xBB,                                 // mov rbx, <read_buf_addr>
        (read_buf_addr >> 0) & 0xFF,
        (read_buf_addr >> 8) & 0xFF,
        (read_buf_addr >> 16) & 0xFF,
        (read_buf_addr >> 24) & 0xFF,
        (read_buf_addr >> 32) & 0xFF,
        (read_buf_addr >> 40) & 0xFF,
        (read_buf_addr >> 48) & 0xFF,
        (read_buf_addr >> 56) & 0xFF,

        0x48, 0xC7, 0xC1, 0x40, 0x00, 0x00, 0x00,   // mov rcx, 64

        0xCD, 0x59,                                 // int 0x59 (read syscall)

        // --- PRINT user input buffer ---
        0x48, 0xBB,                                 // mov rbx, <read_buf_addr>
        (read_buf_addr >> 0) & 0xFF,
        (read_buf_addr >> 8) & 0xFF,
        (read_buf_addr >> 16) & 0xFF,
        (read_buf_addr >> 24) & 0xFF,
        (read_buf_addr >> 32) & 0xFF,
        (read_buf_addr >> 40) & 0xFF,
        (read_buf_addr >> 48) & 0xFF,
        (read_buf_addr >> 56) & 0xFF,

        0xCD, 0x5A,                            // int 0xAD (print syscall)

        // --- OPEN "test.txt" ---
        0x48, 0xBB,                            // mov rbx, <filename_ptr>
        (filename_ptr >> 0) & 0xFF,
        (filename_ptr >> 8) & 0xFF,
        (filename_ptr >> 16) & 0xFF,
        (filename_ptr >> 24) & 0xFF,
        (filename_ptr >> 32) & 0xFF,
        (filename_ptr >> 40) & 0xFF,
        (filename_ptr >> 48) & 0xFF,
        (filename_ptr >> 56) & 0xFF,    
        0xB0, 0x01,                            // mov al, 1 (FA_WRITE | FA_CREATE_ALWAYS)
        0xCD, 0x33,                            // int 0x33 (open syscall)

        // --- WRITE user input to "test.txt" ---
        0x48, 0xBB,                            // mov rbx, <read_buf_addr>
        (read_buf_addr >> 0) & 0xFF,
        (read_buf_addr >> 8) & 0xFF,
        (read_buf_addr >> 16) & 0xFF,
        (read_buf_addr >> 24) & 0xFF,
        (read_buf_addr >> 32) & 0xFF,
        (read_buf_addr >> 40) & 0xFF,
        (read_buf_addr >> 48) & 0xFF,
        (read_buf_addr >> 56) & 0xFF,
        0x48, 0xC7, 0xC1, 0x40, 0x00, 0x00, 0x00,   // mov rcx, 64 (size of input)
        0x48, 0xC7, 0xC2, 0x00, 0x00, 0x00, 0x00,   // mov rdx, 0 (file descriptor)
        0xCD, 0x36,                                 // int 0x36 (write syscall)  

        // --- CLOSE "test.txt" ---
        0x48, 0xC7, 0xC2, 0x00, 0x00, 0x00, 0x00,   // mov rdx, 0 (file descriptor)
        0xCD, 0x34,                                 // int 0x34 (close syscall)      

        // --- PRINT "Data written to test.txt" ---
        0x48, 0xBB,                            // mov rbx, <str_addr>
        (str_addr >> 0) & 0xFF,
        (str_addr >> 8) & 0xFF,
        (str_addr >> 16) & 0xFF,
        (str_addr >> 24) & 0xFF,
        (str_addr >> 32) & 0xFF,
        (str_addr >> 40) & 0xFF,
        (str_addr >> 48) & 0xFF,
        (str_addr >> 56) & 0xFF,
        0xCD, 0x5A,                                 // int 0x5A (print syscall)

        // --- Infinite loop ---
        0xEB, 0xFE                             // jmp $
    };

    memcpy(user_code, user_program, sizeof(user_program));

    return (uint64_t)user_code;
}



void init_user_mode(){
    
    uint64_t code_addr = (uint64_t) create_user_function();
    uint64_t stack_top_addr = ((uint64_t) uheap_alloc(STACK_SIZE, ALLOCATE_STACK)) + STACK_SIZE; // Allocate 16kb for stack and align to page boundary

    printf("Starting Switching to the user mode: code addr.- %x, stack top addr.- %x\n", code_addr, stack_top_addr);

    switch_to_user_mode(stack_top_addr, code_addr);
}





