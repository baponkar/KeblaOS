/*
 
*/

#include "../util/util.h"   // registers_t structure

#include "kshell_helper.h"

#include "../driver/vga/color.h"
#include "../driver/vga/vga_term.h" // clear_screen , color_print

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h" // atof

#include "../memory/detect_memory.h" // Memory management functions

#include "ring_buffer.h"  // Include your ring buffer header

#include "../acpi/descriptor_table/fadt.h" // for acpi_poweroff and acpi_reboot

#include "../process/process.h"
#include "../process/thread.h"

#include "calculator/calculator.h"
#include "steam_locomotive/sl.h" // For locomotive animation
#include "../x86_64/interrupt/apic/apic_interrupt.h" // for test_interrupt

#include "kshell.h"

process_t* kshell_process;
thread_t* kshell_thread;


extern void restore_cpu_state(registers_t* registers);

// Global ring buffer to store keystrokes from the keyboard driver
extern ring_buffer_t* keyboard_buffer;


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
void kshell_prompt() {
    color_print("KeblaOS>> ", COLOR_LIME);
}


void execute_command(char* command) {
    printf("\n");

    if(strcmp(command, "") == 0) {
        return; // Ignore empty commands
    }else if (strcmp(command, "help") == 0) {
        printf("Available commands:\n help, clear, reboot, call print, poweroff, calc, sl, int, features, time, meminfo, pwd, ps, pkill...\n");
    }else if (strcmp(command, "clear") == 0) {
        clear_screen();
    }else if (strcmp(command, "exit") == 0) {
        printf("Exiting shell...\n");
    }else if(strcmp(command, "poweroff") == 0){
        //qemu_poweroff();
        acpi_poweroff();
    }else if(strcmp(command, "reboot") == 0){
        // qemu_reboot();
        acpi_reboot();
    }else if(strcmp(command, "calc") == 0){
        start_calculator();
    }else if(strcmp(command, "sl") == 0){
        // Start locomotive animation
        sl_animation();
    }else if(strcmp(command, "call print") == 0){
        printf("This is a kernel function\n");
    }else if(strcmp(command, "features") == 0){
        print_os_features();
    }else if(strcmp(command, "time") == 0){
        print_current_timestamp();
    }else if(strcmp(command, "meminfo") == 0){
        
    }else if(strcmp(command, "pwd") == 0){
        printf("PID: %d\n", get_current_process()->pid); // Print current process ID
    }else if(strcmp(command, "ps") == 0) {
        print_process_list(); // Function to print the process list
    }else if (strncmp(command, "pkill ", 6) == 0) {
        int pid = atoi(command + 6); // Parse PID after "pkill "
        
        if(pid >= 0 && pid < 100) { // MAX_PROCESSES is your limit
            process_t* proc = get_process_by_pid(pid); // Function to find process by PID            
            if(proc != NULL) {
                printf("Killing process with PID %d...\n", pid);
                delete_process(proc);
            } else {
                printf("No process with PID %d.\n", pid);
            }
        } else {
            printf("Invalid PID: %d\n", pid);
        }
    }else if(strcmp(command, "int") == 0){
        // test_interrupt();
    }else {
        color_print("!Unknown command: ", COLOR_RED);
        color_print(command, COLOR_OLIVE);
        color_print("\nType 'help'\n", COLOR_OLIVE);
    }
}


void run_kshell() {
    char input[BUFFER_SIZE];
    bool kshell_running = true;

    while (kshell_running) {
        kshell_prompt();
        read_command(input, BUFFER_SIZE);
        execute_command(input);
    }
}


void kshell_main() {
    // Initialize the keyboard ring buffer before starting shell operations.
    keyboard_buffer = ring_buffer_init(KEYBOARD_BUF_SIZE);

    if (!keyboard_buffer) {
        printf("Failed to initialize keyboard buffer!\n");
        return;
    }
    
    run_kshell();

    // Cleanup on shell exit
    ring_buffer_free(keyboard_buffer, KEYBOARD_BUF_SIZE);
    while (1) {
        asm volatile("hlt");
    }
}


// Function to create and start the shell process and thread.
void start_kshell() {

    // Create a new process for the shell.
   kshell_process = create_process("Shell Process");
    if (!kshell_process) {
        printf("Failed to create shell process!\n");
        return;
    }

    // Create a new thread within the shell process, with shell_main as its entry.
    kshell_thread = create_thread(kshell_process, "Shell Thread", &kshell_main, NULL);
    if (!kshell_thread) {
        printf("Failed to create shell thread!\n");
        return;
    }

    restore_cpu_state((registers_t *)(uintptr_t)&kshell_thread->registers);

    printf("Shell started successfully.\n");
}







