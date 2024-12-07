
#include "shell.h"

bool shell_running = false;  // Global flag to manage shell state

void shell_prompt() {
    print("KeblaOS>> ");
}


void execute_command(char* command) {
    if (strcmp(command, "help") == 0) {
        print("Available commands: help, clear, reboot, poweroff, time, uptime, regvalue, features, exit.\n");
    } else if (strcmp(command, "clear") == 0) {
        vga_clear();  // Clear the screen using your VGA driver
    } else if (strcmp(command, "reboot") == 0) {
        print("Rebooting...\n");
        // You will need to implement the reboot function depending on your OS
        reboot();
    } else if (strcmp(command, "poweroff") == 0){
        print("Shutting Down!\n");
        print("Please Wait...");
        disableKeyboard();
        disable_cursor();
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
        color_print(">>", COLOR8_BLACK, COLOR8_GREEN);
    }
}


void run_shell(bool is_shell_running) {
    char input[BUFFER_SIZE];

    while (is_shell_running) {
        shell_prompt();  // Display shell prompt
        read_command(input);  // Function to read user input (can be implemented based on your keyboard handler)
        execute_command(input);  // Process the input command
    }
}


void poweroff() {
    outw(0x604, 0x2000);  // Write to ACPI power-off port (used by Bochs/QEMU)
}


void reboot(){
    outb(0x64, 0xFE);  // Send reset command to the keyboard controller
}




// Define a structure to match the order of registers in the assembly function
struct registers {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
};

// Implement the print_registers_c function
void print_registers_c(struct registers* regs) {
    printf("Register Values:\n");
    printf("RAX: %x\n", regs->rax);
    printf("RBX: %x\n", regs->rbx);
    printf("RCX: %x\n", regs->rcx);
    printf("RDX: %x\n", regs->rdx);
    printf("RSI: %x\n", regs->rsi);
    printf("RDI: %x\n", regs->rdi);
    printf("RBP: %x\n", regs->rbp);
    printf("R8 : %x\n", regs->r8);
    printf("R9 : %x\n", regs->r9);
    printf("R10: %x\n", regs->r10);
    printf("R11: %x\n", regs->r11);
    printf("R12: %x\n", regs->r12);
    printf("R13: %x\n", regs->r13);
    printf("R14: %x\n", regs->r14);
    printf("R15: %x\n", regs->r15);
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


