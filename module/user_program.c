


__attribute__((section(".text.user")))
void _start() {
    
    const char *msg = "Hello from User Program\n";

    asm volatile (
        "mov %%rbx, %0\n"
        "mov $0xad, %%rax\n"
        "int $0xad\n"
        "jmp .\n"
        :
        : "r"(msg)
        : "rax", "rbx"
    );
}

