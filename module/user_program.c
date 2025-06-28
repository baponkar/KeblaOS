
const char *msg = "Hello from User Program\n";

__attribute__((section(".text")))
void _start() {

    asm volatile (
        "mov %[msg], %%rbx\n"
        "int $0x5A\n"           // Trigger print syscall
        "1:\n"
        "jmp 1b\n"             // Infinite loop
        :
        : [msg] "r" (msg)
        : "rbx"
    );

}

