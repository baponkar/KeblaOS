/*
Version: 1.0
Author: Baponkar
Description: A simple user program that prints a message using the interrupt-based system call mechanism.
*/
const char *msg = "Hello from User Program\n";

__attribute__((section(".text")))
void _start() {

    asm volatile (
        "mov %[msg], %%rbx\n"
        "int $0x5A\n"           // Trigger print syscall
        "1:\n"
        "jmp 1b\n"              // Infinite loop
        :
        : [msg] "r" (msg)
        : "rbx"
    );

}

