#include "../driver/vga.h"  // Assuming you have a VGA text driver.
#include "../driver/ports.h"
#include "../lib/string.h"    // Assuming you have string manipulation functions.
#include "../driver/keyboard.h"  // Assuming you have keyboard input handling.
#include "../x86_64/gdt/gdt.h"
#include "../util/util.h"

#include "../x86_64/idt/idt.h"
#include "../kernel/kernel.h"

#include "../x86_64/rtc/rtc.h"

#include "../driver/image_data.h"

#include "../bootloader/acpi.h"
#include "../bootloader/boot.h"

#include "../pcb/process.h"

#include "shell.h"



#define INPUT_BUFFER_SIZE 128

static process_t *shell_process = NULL; // Global reference to the shell process
bool shell_running = false;

void shell_prompt() {
    print("KeblaOS>> ");
}

void execute_command(char* command) {
    if (strcmp(command, "help") == 0) {
        print("Available commands:\n");
        print("[ help, clear, reboot, poweroff, bootinfo, time, uptime, regvalue, checkgdt, features, exit ]\n");
    } else if (strcmp(command, "clear") == 0) {
        clear_screen();
    } else if (strcmp(command, "reboot") == 0) {
        print("Rebooting...\n");
        qemu_reboot();
    } else if (strcmp(command, "poweroff") == 0) {
        print("Shutting Down...\n");
        print("Please Wait...");
        qemu_poweroff();
    } else if (strcmp(command, "exit") == 0) {
        print("Exiting shell...\n");
        shell_running = false; // End the shell process
    } else if (strcmp(command, "") == 0) {
        print("type 'help'\n");
    } else {
        print("!Unknown command: ");
        print(command);
        print("\n");
        print("type 'help'\n");
    }
}

void shell_main() {
    char input[INPUT_BUFFER_SIZE];
    size_t input_index = 0;

    shell_running = true;
    print("KeblaOS Shell started. Type 'help' for commands.\n");

    while (shell_running) {
        shell_prompt();
        input_index = 0;

        // Read user input until Enter key is pressed
        while (true) {
            char key = keyboard_get_char();
            if (key == '\n') {
                input[input_index] = '\0'; // Null-terminate the input
                print("\n");              // Move to the next line
                break;
            } else if (key == '\b') { // Handle backspace
                if (input_index > 0) {
                    input_index--;
                    print("\b \b"); // Erase character on the screen
                }
            } else if (key >= 32 && key <= 126) { // Printable ASCII characters
                if (input_index < INPUT_BUFFER_SIZE - 1) {
                    input[input_index++] = key;
                    char str[2] = {key, '\0'};
                    print(str); // Echo the character
                }
            }
        }

        execute_command(input); // Process the entered command
    }

    print("Shell process terminated.\n");
    terminate_process(shell_process); // Terminate the shell process
}
