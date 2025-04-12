/*
Switch into User Mode

References:
    https://github.com/dreamportdev/Osdev-Notes/blob/master/06_Userspace/02_Switching_Modes.md
    https://wiki.osdev.org/Getting_to_Ring_3
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


extern void start_kshell();     // Defined in kshell.c
// extern void user_stub();        // Defined in user_stub.asm
// extern void user_stub_end();    // Defined in user_stub.asm 
extern void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr);   // Defined in switch_to_user_mode.asm


uint64_t create_user_function() {

    // uint64_t user_stub_size = ((uint64_t)&user_stub_end - (uint64_t)&user_stub);

    // uint64_t user_addr = (uint64_t) uheap_alloc(user_stub_size);    // Allocate a page in user space
    uint64_t user_addr = (uint64_t) uheap_alloc(0x1000);
    if (!user_addr) return 0;

    // memcpy((void *)user_addr, (void *)&user_stub, user_stub_size);  // Copy function code
    memcpy((void *)user_addr, (void *)&start_kshell, 0x1000);

    return user_addr; // Return the user-accessible function pointer
}


void init_user_mode(){
    uint64_t stack_base_addr = ((uint64_t) uheap_alloc(STACK_SIZE));
    for(uint64_t addr = stack_base_addr; addr < stack_base_addr + STACK_SIZE; addr += 0x1000){
        page_t *_page = get_page(addr, 0,  (pml4_t *)get_cr3_addr());
        _page->nx = 1;  // Making non-executable
        _page->rw = 1;  // Making it read-writable
    }
    
    uint64_t stack_top_addr = stack_base_addr + STACK_SIZE;   // Set stack top

    uint64_t code_addr = create_user_function();
    page_t *code_page = get_page(code_addr, 0, (pml4_t *)get_cr3_addr());
    code_page->rw = 0; // Making readable only
    code_page->nx = 0;  // Making executable
    
    flush_tlb_all();


    // printf("Checking stack_addr:\n");
    // page_t *stack_page = get_page(stack_base_addr, 0,  (pml4_t *)get_cr3_addr());
    // debug_page(stack_page);
    // printf("\n");

    // printf("Checking code_addr:\n");
    // debug_page(code_page);
    // printf("\n");
    
    printf("Starting Switching to the user mode\n");
    switch_to_user_mode(stack_top_addr, code_addr);
}






