
#include "shell.h"

bool shell_running = false;  // Global flag to manage shell state

void shell_prompt() {
    print("KeblaOS>> ");
}


void execute_command(char* command) {
    
    if (strcmp(command, "help") == 0) {
        print("Available commands: help, clear, reboot, poweroff, time, uptime, regvalue,check gdt, features, test interrupt, exit.\n");
    } else if (strcmp(command, "clear") == 0) {
        clear_screen();  // Clear the screen using your VGA driver
    } else if (strcmp(command, "reboot") == 0) {
        print("Rebooting...\n");
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
        print("Test Interrupts\n");
        test_interrupt();
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
    outb(0x64, 0xFE);   // Send reset command to the keyboard controller
}



// This C function will print the register values passed via the stack.
void print_registers_c(uint64_t rdi, uint64_t rsi, uint64_t rbp, uint64_t rsp, 
                       uint64_t rbx, uint64_t rdx, uint64_t rcx, uint64_t rax,
                       uint64_t r8,  uint64_t r9,  uint64_t r10, uint64_t r11, 
                       uint64_t r12, uint64_t r13, uint64_t r14, uint64_t r15 ) {
    print("Register Values:\n");
    
    print("RAX: ");
    print_hex(rax);
    print("\n");

    print("RBX: ");
    print_hex(rbx);
    print("\n");

    print("RCX: ");
    print_hex(rcx);
    print("\n");

    print("RDX: ");
    print_hex(rdx);
    print("\n");

    print("RSI: ");
    print_hex(rsi);
    print("\n");

    print("RDI: ");
    print_hex(rdi);
    print("\n");

    print("RBP: ");
    print_hex(rbp);
    print("\n");

    print("R8: ");
    print_hex(r8);
    print("\n");

    print("R9: ");
    print_hex(r9);
    print("\n");

    print("R10: ");
    print_hex(r10);
    print("\n");

    print("R11: ");
    print_hex(r11);
    print("\n");

    print("R12: ");
    print_hex(r12);
    print("\n");

    print("R13: ");
    print_hex(r13);
    print("\n");

    print("R14: ");
    print_hex(r14);
    print("\n");

    print("R15: ");
    print_hex(r15);
    print("\n");
}


void print_features(){
    print("Features:\n");
    print("Architecture : x86_64.\n");
    print("1. Limine Bootloading.\n");
    print("2. GDT initialization.\n");
    print("3. VGA Graphics Driver.\n");
    print("4. IDT initialization.\n");
    print("5. Keyboard driver initialization.\n");
    print("6. PIT Timer initialization.\n");
    print("7. Basic User Shell");
    print("8. Memory Management Unit(Kheap, PMM, 4 Level Paging)\n");
    print("7. Standard Libraries : match.h, stddef.h, stdint.h, stdio.h, stdlib.h, string.h\n");
}


