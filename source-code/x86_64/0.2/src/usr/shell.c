
#include "shell.h"

bool shell_running = false;  // Global flag to manage shell state

void shell_prompt() {
    print("KeblaOS>> ");
}


void execute_command(char* command) {
    if (strcmp(command, "help") == 0) {
        print("Available commands: help, clear, reboot, poweroff, time, uptime, regvalue, features, exit.\n");
    } else if (strcmp(command, "clear") == 0) {
        cls();  // Clear the screen using your VGA driver
    } else if (strcmp(command, "reboot") == 0) {
        print("Rebooting...\n");
        // You will need to implement the reboot function depending on your OS
        reboot();
    } else if (strcmp(command, "poweroff") == 0){
        print("Shutting Down!\n");
        print("Please Wait...");
        poweroff();
    } else if (strcmp(command, "time") == 0){
        print("Current Time :\n");
        // printing current time
    } else if (strcmp(command, "uptime") == 0){
        print("Up time : \n");
        // printing the time run this os
    }else if (strcmp(command, "regvalue") == 0){
        print_registers();
    }else if (strcmp(command, "check gdt") == 0){
        check_gdt();
    }else if (strcmp(command, "features") == 0){
        print("Following features have implemented:\n");
        print_features();
    }else if (strcmp(command, "test interrupt") == 0){
        printf("Test Interrupts\n");
        void test_interrupt();
    } else if (strcmp(command, "exit") == 0) {
        print("Exiting shell...\n");
        shell_running = false;
    }else {
        print("Unknown command: ");
        print(command);
        print("\n");
        print("type 'help'\n");
    }

    if(!strcmp(command, "poweroff") == 0){
        print(">>");
    }
}


void run_shell(bool is_shell_running) {
    char input[BUFFER_SIZE];

    while (is_shell_running) {
        shell_prompt();  // Display shell prompt
        //read_command(input);  // Function to read user input (can be implemented based on your keyboard handler)
        //execute_command(input);  // Process the input command
    }
}


void poweroff() {
    outw(0x604, 0x2000);  // Write to ACPI power-off port (used by Bochs/QEMU)
}


void reboot(){
    outb(0x64, 0xFE);  // Send reset command to the keyboard controller
}



// This C function will print the register values passed via the stack.
void print_registers_c(uint64_t rdi, uint64_t rsi, uint64_t rbp, uint64_t rsp, 
                       uint64_t rbx, uint64_t rdx, uint64_t rcx, uint64_t rax,
                       uint64_t r8, uint64_t r9, uint64_t r10, uint64_t r11, uint64_t r12, uint64_t r13, uint64_t r14, uint64_t r15 ) {
    printf("Register Values:\n");
    
    printf("RAX: ");
    print_hex(rax);
    printf("\n");

    printf("RBX: ");
    print_hex(rbx);
    printf("\n");

    printf("RCX: ");
    print_hex(rcx);
    printf("\n");

    printf("RDX: ");
    print_hex(rdx);
    printf("\n");

    printf("RSI: ");
    print_hex(rsi);
    printf("\n");

    printf("RDI: ");
    print_hex(rdi);
    printf("\n");

    printf("RBP: ");
    print_hex(rbp);
    printf("\n");

    printf("R8: ");
    print_hex(r8);
    printf("\n");

    printf("R9: ");
    print_hex(r9);
    printf("\n");

    printf("R10: ");
    print_hex(r10);
    printf("\n");

    printf("R11: ");
    print_hex(r11);
    printf("\n");

    printf("R12: ");
    print_hex(r12);
    printf("\n");

    printf("R13: ");
    print_hex(r13);
    printf("\n");

    printf("R14: ");
    print_hex(r14);
    printf("\n");

    printf("R15: ");
    print_hex(r15);
    printf("\n");
}


void print_features(){
    printf("Features:\n");
    printf("1. GRUB2 Bootloading\n");
    printf("2. Keyboard Driver\n");
    printf("3. VGA Text Mode Display Driver\n");
    printf("4. GDT initialization\n");
    printf("5. IDT initialization\n");
    printf("6. PIT initialization\n");
    printf("7. Basic User Shell");
    printf("7. Standard Libraries : match.h, stddef.h, stdint.h, stdio.h, stdlib.h, string.h\n");
    printf("_________________________________________________________________________________\n");
}


