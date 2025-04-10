
#include "../memory/umalloc.h"
#include "../memory/uheap.h"
#include "../memory/paging.h"
#include "../util/util.h"
#include  "../lib/stdio.h"
#include  "../lib/string.h"
#include "../kshell/kshell.h"

#include "syscall.h"

#include "switch_to_user.h"


#define KERNEL_CS 0x08 | 0
#define KERNEL_SS 0x10 | 0

#define USER_CS 0x18 | 3    // 0x1B
#define USER_SS 0x20 | 3    // 0x23

#define STACK_SIZE 0x4000   // 16 kb

uint64_t stack_addr;
uint64_t code_addr;

extern uint64_t user_stub(uint64_t syscall_num, uint64_t arg1, uint64_t arg2);
extern void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr);



uint64_t create_user_function() {
    uint64_t user_addr = (uint64_t) uheap_alloc(0x1000); // Allocate a page in user space
    if (!user_addr) return 0;
    // printf("Created a userspace pointer at: %x\n", user_addr);

    memcpy((void *)user_addr, (void *)&user_stub, 0x1000); // Copy function code
    // printf("memory copied from kernel function: %x into user function: %x\n", (uint64_t)&user_stub, (uint64_t) user_addr);

    return user_addr; // Return the user-accessible function pointer
}


void init_user_mode(){
    stack_addr = (uint64_t) uheap_alloc(STACK_SIZE);
    code_addr = (uint64_t) create_user_function();

    printf("Starting Switching to the user mode\n");
    switch_to_user_mode(stack_addr, code_addr);
    printf("Successfully Switch To User_mode implemented\n");
}



