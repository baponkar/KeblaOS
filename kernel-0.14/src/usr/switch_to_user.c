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


extern void user_main();


// uint64_t create_user_function() {

//     uint64_t user_stub_size = ((uint64_t)&user_stub_end - (uint64_t)&user_stub);
//     uint64_t user_addr = (uint64_t) uheap_alloc(user_stub_size);    // Allocate a page in user space
//     if (!user_addr) return 0;
//     memcpy((void *)user_addr, (void *)&user_stub, user_stub_size);  // Copy function code

//     return user_addr; // Return the user-accessible function pointer
// }


void init_user_mode(){
    uint64_t stack_base_addr = ((uint64_t) uheap_alloc(STACK_SIZE));
    for(uint64_t addr = stack_base_addr; addr < stack_base_addr + STACK_SIZE; addr += 0x1000){
        page_t *_page = get_page(addr, 0,  (pml4_t *)get_cr3_addr());
        _page->nx = 1;  // Making non-executable
        _page->rw = 1;  // Making it read-writable
    }
    
    uint64_t stack_top_addr = stack_base_addr + STACK_SIZE;   // Set stack top

    uint64_t code_addr = (uint64_t)&user_main;
    page_t *code_page = get_page(code_addr, 0, (pml4_t *)get_cr3_addr());
    code_page->rw = 0; // Making readable only
    code_page->nx = 0;  // Making executable
    
    flush_tlb_all();
    
    printf("Starting Switching to the user mode: code addr.- %x, stack addr.- %x\n", code_addr, stack_top_addr);
    // switch_to_user_mode_1(stack_top_addr, code_addr);
    // switch_to_user_mode(stack_top_addr, code_addr);
    enter_user_mode(0x400000, stack_top_addr);
}


int is_user_mode() {
    uint64_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    return (cs & 0x3) == 3; // If CPL (bits 0-1) == 3, then it's user mode
}


__attribute__((naked, noreturn))
void switch_to_user_mode_1(uint64_t stack_addr, uint64_t code_addr)
{
    asm volatile(" \
        push $0x23 \n\
        push %0 \n\
        push $0x202 \n\
        push $0x1B \n\
        push %1 \n\
        sti \n\
        iretq \n\
        " :: "r"(stack_addr), "r"(code_addr));
}



__attribute__((naked, noreturn))
void enter_user_mode(uint64_t rip, uint64_t rsp) {
    asm volatile (
        "cli             \n"
        "mov $0x23, %%ax \n"
        "mov %%ax, %%ds  \n"
        "mov %%ax, %%es  \n"
        "mov %%ax, %%fs  \n"
        "mov %%ax, %%gs  \n"

        "push $0x23     \n"      // user SS
        "push %[rsp]    \n"      // user RSP
        "push 0x202     \n"            
        "push $0x1B     \n"      // user CS
        "push %[rip]    \n"      // user RIP
        "iretq\n"
        :
        : [rip]"r"(rip), [rsp]"r"(rsp)
        : "memory"
    );
}









