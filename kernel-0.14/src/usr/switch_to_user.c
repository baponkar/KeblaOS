/*
Switch into User Mode

References:
    https://github.com/dreamportdev/Osdev-Notes/blob/master/06_Userspace/02_Switching_Modes.md
    https://wiki.osdev.org/Getting_to_Ring_3
    https://hasinisama.medium.com/operating-system-user-mode-2c2f19ca8e43
    https://hasinisama.medium.com/operating-systems-the-road-to-user-mode-854b72221b1b
*/
#include "../memory/umalloc.h"
#include "../memory/uheap.h"
#include "../memory/paging.h"
#include "../util/util.h"
#include  "../lib/stdio.h"
#include  "../lib/string.h"
#include "../kshell/kshell.h"
#include "../syscall/syscall_manager.h"

#include "switch_to_user.h"


#define KERNEL_CS 0x08  // 0x08 | 0
#define KERNEL_SS 0x10  // 0x10 | 0
#define USER_CS 0x1B    // 0x18 | 3
#define USER_SS 0x23    // 0x20 | 3 = 100000 | 11 = 100011 = 0x23

#define STACK_SIZE 0x4000   // 16 kb



int is_user_mode() {
    uint64_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    return (cs & 0x3) == 3;     // If CPL (bits 0-1) == 3, then it's user mode
}

__attribute__((naked, noreturn))
void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr)
{
    asm volatile(
        "cli\n"                     // Disable Interrupt
        "mov $0x23, %%ax    \n"     // User-mode data segment (ring 3)
        "mov %%ax, %%ds     \n"     // Pushing data segment
        "mov %%ax, %%es     \n"     // 
        "mov %%ax, %%fs     \n"     //
        "mov %%ax, %%gs     \n"     //
        "pushq $0           \n"     // Align stack (optional dummy value)
        "pushq $0x23        \n"     // Push USER_SS(DATA SEGMENT) Selector
        "pushq %[stack]     \n"     // Push User Stack pointer
        "pushfq             \n"     // Push RFLAGS
        "popq %%rax         \n"     // Taking RFLAGS into RAX
        "or $0x200, %%rax   \n"     // Set IF flag to make automatic enabling Interrupt
        "pushq %%rax        \n"     // Push Updated RFLAGS into the stack
        "pushq $0x1B        \n"     // Push USER_CS(CODE SEGMENT)
        "pushq %[entry]     \n"     // Push entry point address        
        "iretq              \n"     // Interrupt Return to User Mode
        :
        : [stack] "r" (stack_addr), [entry] "r" (code_addr)
        : "rax"
    );
}


uint64_t create_user_function() {
    void *user_code = (void *) uheap_alloc(0x1000);

    // simple infinite loop machine code
    uint8_t user_program[] = {
        0xeb, 0xfe  // Infinite loop: jmp $
    };

    memcpy(user_code, (void*)&user_program, sizeof(user_program));

    return (uint64_t)user_code; // Return the user-accessible function pointer
}


void init_user_mode(){
    uint64_t code_addr = (uint64_t) create_user_function();
    page_t *code_page = get_page(code_addr, 0, (pml4_t *)get_cr3_addr());
    code_page->rw = 0;      // Making readable only
    code_page->nx = 0;      // Making executable
    code_page->user = 1;    // Making User accessible

    uint64_t stack_base_addr = ((uint64_t) uheap_alloc(STACK_SIZE));
    for(uint64_t addr = stack_base_addr; addr < stack_base_addr + STACK_SIZE; addr += 0x1000){
        page_t *_page = get_page(addr, 0,  (pml4_t *)get_cr3_addr());
        _page->rw = 1;      // Making it read-writable
        _page->nx = 1;      // Making non-executable
        _page->user = 1;    // Making User accessible
    }
    uint64_t stack_top_addr = stack_base_addr + STACK_SIZE;   // Set stack top
    
    flush_tlb_all();
    
    printf("Starting Switching to the user mode: code addr.- %x, stack addr.- %x\n", code_addr, stack_top_addr);
    switch_to_user_mode(stack_top_addr, code_addr);
}





