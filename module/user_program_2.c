/*
Version: 2.0
Author: Baponkar
Description: User program that demonstrates various system calls including printing, reading from keyboard, file operations,
             and directory operations in KeblaOS.
*/

char  *msg = "Hello from User Program 2\n";
char  *file_content = "This is a test file content written by User Program 2.\n";
char  *file_name = "test_file.txt";

int read = 0x01;
int write = 0x02;
int one_existing = 0x00;
int create_new = 0x04;
int create_always = 0x08;
int open_always = 0x10;
int append = 0x30;

__attribute__((section(".text")))

void _start() {
    asm volatile (

        // Print message
        "mov %[msg], %%rbx\n"
        "int $0x5A\n"           

        // Test mounting FatFs
        "int $0x52\n"            // Trigger mount syscall
        "int $0x5C\n"            // Print rax value : rax: 0xFFFF800000017000

        // Create a file
        "mov %[file_name], %%rbx\n"
        "mov $0x08, %%rcx\n"     // FA_CREATE_ALWAYS
        "int $0x33\n"            // Trigger open syscall with FA_CREATE_ALWAYS
        "int $0x5C\n"            // Print rax value : rax: 0xFFFF800000019000

        // Open the file
        "mov %[file_name], %%rbx\n"
        "mov $0x10, %%rcx\n"     // FA_OPEN_ALWAYS
        "int $0x33\n"            // Trigger open syscall with FA_OPEN_ALWAYS
        "int $0x5C\n"            // Print rax value : rax: 0xFFFF80000001B000

        // Write to the file
        // "mov %%rax, %%rbx\n"
        // "mov $0x02, %%rcx\n"     // FA_WRITE
        // "mov %[file_content], %%rdx\n"
        // "mov $0x40, %%rdi\n"     // Size of file_content
        // "int $0x36\n"            // Trigger write syscall
        // "int $0x5C\n"            // Print rax value : rax: 


        // Read from the file
   

        "1:\n"
        "jmp 1b\n"              // Infinite loop
        :
        : [msg] "r" (msg),
          [file_name] "r" (file_name),
          [file_content] "r" (file_content)

        : "rbx", "rax", "rcx"
    );

}
