
#include "../memory/umalloc.h"
#include "../memory/uheap.h"
#include "../memory/paging.h"
#include "../util/util.h"
#include  "../lib/stdio.h"
#include  "../lib/string.h"
#include "../kshell/kshell.h"

#include "switch_to_user.h"


#define KERNEL_CS 0x08 | 0
#define KERNEL_SS 0x10 | 0
#define USER_CS 0x18 | 3    // 0x1B
#define USER_SS 0x20 | 3    // 0x23

#define STACK_SIZE 0x4000   // 16 kb

uint64_t stack_addr;
uint64_t code_addr;

extern void user_stub();
extern void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr);

void kernel_function() {
    printf("Executing from user-accessible memory!\n");
    while(1){
        asm volatile("hlt");
    }
}

void *create_user_function() {
    void *user_addr = (void *) uheap_alloc(0x1000); // Allocate a page in user space
    if (!user_addr) return NULL;

    memcpy(user_addr, (void *)&user_stub, 0x1000); // Copy function code

    return user_addr; // Return the user-accessible function pointer
}


void init_user_mode(){
    stack_addr = (uint64_t) uheap_alloc(STACK_SIZE);
    code_addr = (uint64_t) create_user_function();

    
    printf("stack_addr: %x, code_addr: %x\n", stack_addr, code_addr);
    // switch_to_user_mode(stack_addr, code_addr);
    printf("Starting Switching to the user mode\n");
    switch_to_user_mode( stack_addr, code_addr);
    printf("Successfully Switch To User_mode implemented\n");
}






void check_usermode() {
    uint64_t cs;
    asm volatile("mov %%cs, %0" : "=r" (cs)); // Read CS register

    if ((cs & 3) == 3) {
        printf("User mode is active! (CPL = 3)\n");
    } else {
        printf("Still in kernel mode! (CPL = 0)\n");
    }
}
