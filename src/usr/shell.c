/*
 
*/

#include "../util/util.h"   // registers_t structure


#include "../driver/vga/color.h"
#include "../driver/vga/vga_term.h" // clear_screen , color_print

#include "../lib/stdio.h"
#include "../lib/string.h"

#include "ring_buffer.h"  // Include your ring buffer header

#include "../acpi/descriptor_table/fadt.h" // for acpi_poweroff and acpi_reboot

#include "../process/process.h"
#include "../process/thread.h"

#include "shell.h"

#define BUFFER_SIZE 256       // For the complete command
#define KEYBOARD_BUF_SIZE 128 // Ring buffer capacity for keystrokes

// Global ring buffer to store keystrokes from the keyboard driver
ring_buffer_t* keyboard_buffer;


// Modified read_command that reads from the ring buffer.
void read_command(char *command, size_t bufsize) {
    size_t index = 0;
    uint8_t ch;

    // Loop until we either fill the command buffer or encounter Enter
    while (index < bufsize - 1) {
        // Wait for a character to be available
        while (is_ring_buffer_empty(keyboard_buffer)) {
            // Optionally, you could yield the CPU or sleep here
        }
        // Pop a character from the ring buffer.
        if (ring_buffer_pop(keyboard_buffer, &ch) == 0) {
            // For this example, we assume Enter sends a newline ('\n')
            if (ch == '\n' || ch == '\r') {
                break;
            }
            command[index++] = ch;
        }
    }
    command[index] = '\0';
}

// Rest of your shell.c implementation remains largely the same.

void shell_prompt() {
    color_print("KeblaOS>> ", COLOR_LIME);
}

void execute_command(char* command) {
    printf("\nKeblaOS>> ");
    if (strcmp(command, "help") == 0) {
        printf("Available commands: help, clear, reboot, poweroff ...\n");
    } else if (strcmp(command, "clear") == 0) {
        // Implement clear_screen() as needed.
        clear_screen();
    }
    // ... handle other commands
    else if (strcmp(command, "exit") == 0) {
        printf("Exiting shell...\n");
        // Set global flag to stop shell loop
    }else if(strcmp(command, "poweroff") == 0){
        printf("Powering Off!\n");
        //qemu_poweroff();
        acpi_poweroff();
    }else if(strcmp(command, "reboot") == 0){
        printf("Rebooting...\n");
        //qemu_poweroff();
        acpi_reboot();
    }else {
        printf("!Unknown command: %s\n", command);
        printf("Type 'help'\n");
    }
}

void run_shell() {
    char input[BUFFER_SIZE];
    bool shell_running = true;

    while (shell_running) {
        shell_prompt();
        read_command(input, BUFFER_SIZE);
        execute_command(input);
    }
}

void shell_main() {
    // Initialize the keyboard ring buffer before starting shell operations.
    keyboard_buffer = ring_buffer_init(KEYBOARD_BUF_SIZE);

    if (!keyboard_buffer) {
        printf("Failed to initialize keyboard buffer!\n");
        return;
    }
    
    run_shell();

    // Cleanup on shell exit
    ring_buffer_free(keyboard_buffer, KEYBOARD_BUF_SIZE);
    while (1) {
        asm volatile("hlt");
    }
}

extern void restore_cpu_state(registers_t* registers);
// Function to create and start the shell process and thread.
void start_shell() {

    // Create a new process for the shell.
    process_t* shell_process = create_process("Shell Process");
    if (!shell_process) {
        printf("Failed to create shell process!\n");
        return;
    }

    // Create a new thread within the shell process, with shell_main as its entry.
    thread_t* shell_thread = create_thread(shell_process, "Shell Thread", &shell_main, NULL);
    if (!shell_thread) {
        printf("Failed to create shell thread!\n");
        return;
    }

    restore_cpu_state((registers_t *)(uintptr_t)&shell_thread->registers);

    printf("Shell started successfully.\n");
}
