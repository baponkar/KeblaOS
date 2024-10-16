
#include "shell.h"


void shell_prompt() {
    print("KeblaOS>> ");
}

void execute_command(char* command) {
    if (strcmp(command, "help") == 0) {
        print("Available commands: help, clear, reboot, poweroff.\n");
    } else if (strcmp(command, "clear") == 0) {
        vga_clear();  // Clear the screen using your VGA driver
    } else if (strcmp(command, "reboot") == 0) {
        print("Rebooting...\n");
        // You will need to implement the reboot function depending on your OS
        reboot();
    } else if (strcmp(command, "poweroff") == 0){
        print("Shutting Down!\n");
        poweroff();
    } else {
        print("Unknown command: ");
        print(command);
        print("\n");
    }
}

void shell() {
    char command[BUFFER_SIZE];

    while (1) {
        shell_prompt();
        read_input(command, BUFFER_SIZE);  // read_input is a function to read from keyboard
        execute_command(command);
    }
}

void poweroff() {
    outw(0x604, 0x2000);  // Write to ACPI power-off port (used by Bochs/QEMU)
}

void reboot(){
    outb(0x64, 0xFE);  // Send reset command to the keyboard controller
}



