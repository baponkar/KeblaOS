#include "syscall.h"

#define SYS_WRITE 1
#define SYS_READ  2
#define SYS_EXIT  3

void syscall_handler(registers_t *regs) {
    uint32_t syscall_num = regs->eax;
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


int sys_write(const char *str) {
    int result;
    asm volatile (
        "movl $1, %%eax;"      // Syscall number for write # 1
        "movl %1, %%ebx;"      // First argument (string)
        "int $128;"           // Trigger interrupt 128
        "movl %%eax, %0;"      // Return result in EAX
        : "=r"(result) : "r"(str)
    );
    return result;
}


int sys_read(const char *str) {
    int result;
    asm volatile (
        "movl $2, %%eax;"      // Syscall number for read # 2
        "movl %1, %%ebx;"      // First argument (string)
        "int $177;"           // Trigger interrupt 177
        "movl %%eax, %0;"     // Return result in EAX
        : "=r"(result) : "r"(str)
    );
    return result;
}

void syscall_check(){
    sys_read("Hello World!");
    sys_write("Hello World!");
}

