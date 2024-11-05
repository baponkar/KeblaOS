#include "syscall.h"

#define SYS_WRITE 1
#define SYS_READ  2
#define SYS_EXIT  3

void syscall_handler(registers_t *regs) {
    uint64_t syscall_num = regs->rax;
    print("Interrupt No :");
    print_dec(regs->int_no);
    print("\n");
    switch (syscall_num) {
        case SYS_WRITE:
            // Call your write function
            print("Write System call\n");
            break;
        case SYS_READ:
            // Call your read function
            print("Read System call\n");
            break;
        case SYS_EXIT:
            // Handle exit
            print("Exit System call\n");
            break;
        default:
            // Unknown system call
            print("Default System Call\n");
            break;
    }
}




void syscall_check(){
    sys_read("Hello World!");
    sys_write("Hello World!");
}

