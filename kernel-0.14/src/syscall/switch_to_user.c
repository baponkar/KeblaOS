
#include "../memory/umalloc.h"
#include "../memory/uheap.h"
#include "../memory/paging.h"
#include "../util/util.h"
#include  "../lib/stdio.h"
#include  "../lib/string.h"
#include "../kshell/kshell.h"
#include "syscall_manager.h"
#include "../usr/user_shell.h"

#include "switch_to_user.h"


#define KERNEL_CS 0x08 | 0
#define KERNEL_SS 0x10 | 0

#define USER_CS 0x18 | 3    // 0x1B
#define USER_SS 0x20 | 3    // 0x23

#define STACK_SIZE 0x4000   // 16 kb


extern void user_stub();        // Defined in user_stub.asm
extern void user_stub_end();    // Defined in user_stub.asm 
extern void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr);   // Defined in switch_to_user_mode.asm




uint64_t create_user_function() {

    uint64_t user_stub_size = ((uint64_t)&user_stub_end - (uint64_t)&user_stub);

    uint64_t user_addr = (uint64_t) uheap_alloc(user_stub_size);    // Allocate a page in user space
    if (!user_addr) return 0;

    memcpy((void *)user_addr, (void *)&user_stub, user_stub_size);  // Copy function code

    return user_addr; // Return the user-accessible function pointer
}


void init_user_mode(){
    uint64_t stack_addr = (uint64_t) uheap_alloc(STACK_SIZE) + STACK_SIZE;
    uint64_t code_addr = create_user_function();

    printf("Starting Switching to the user mode\n");
    switch_to_user_mode(stack_addr, code_addr);
}




