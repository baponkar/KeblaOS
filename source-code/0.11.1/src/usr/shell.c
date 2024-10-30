
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
    print(">>");
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



// This C function will print the register values passed via the stack.
void print_registers_c(uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t esp, 
                       uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax) {
    printf("Register Values:\n");
    
    printf("EAX: ");
    print_hex(eax);
    printf("\n");

    printf("EBX: ");
    print_hex(ebx);
    printf("\n");

    printf("ECX: ");
    print_hex(ecx);
    printf("\n");

    printf("EDX: ");
    print_hex(edx);
    printf("\n");

    printf("ESI: ");
    print_hex(esi);
    printf("\n");

    printf("EDI: ");
    print_hex(edi);
    printf("\n");

    printf("EBP: ");
    print_hex(ebp);
    printf("\n");

    printf("ESP: ");
    print_hex(esp);
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
    printf("____________________________________________________________________________\n");

}


void test_interrupt(){
    // Testing Interrupts
    asm volatile ("int $0x3");
    asm volatile ("int $0x0");
    asm volatile ("int $0x20");
    print_dec( 104 / 0 );
}