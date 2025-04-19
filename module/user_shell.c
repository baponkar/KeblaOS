

/* Making Machine Code

/usr/local/x86_64-elf/bin/x86_64-elf-ld -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000 -T user_linker_x86_64.ld module/user_shell.o --oformat=i386:x86-64 -o module/user_shell.elf
/usr/local/x86_64-elf/bin/x86_64-elf-objcopy -O binary module/user_shell.elf module/user_shell.bin
objdump -D -b binary -m i386:x86-64 module/user_shell.bin
*/
#include "user_shell.h"

#define INT_SYSCALL_READ   172
#define INT_SYSCALL_PRINT  173


void syscall_print(const char *str) {
    asm volatile (
        "movq $0x02, %%rax\n\t"
        "movq %0, %%rbx\n\t"
        "int $0x80\n\t"
        :
        : "r" ((uint64_t)str)
        : "rax", "rbx"
    );
}

int syscall_read(char *buf, int size) {
    int ret;
    asm volatile (
        "movq $0x01, %%rax\n\t"
        "movq %1, %%rbx\n\t"
        "movq %2, %%rcx\n\t"
        "int $0x80\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(ret)
        : "r" ((uint64_t)buf), "r" ((uint64_t)size)
        : "rax", "rbx", "rcx"
    );
    return ret;
}


void user_shell_main() {
    char buffer[128];
    while (1) {
        syscall_print("kebla> "); // prompt

        int len = syscall_read(buffer, sizeof(buffer) - 1);
        buffer[len] = '\0'; // null-terminate

        syscall_print("You typed: ");
        syscall_print(buffer);
        syscall_print("\n");
    }
}




