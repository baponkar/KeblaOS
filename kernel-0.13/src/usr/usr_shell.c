/*

*/

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../syscall/syscall.h"
#include "../process/process.h"
#include "../process/thread.h"
#include "../kshell/ring_buffer.h"

#include "user_shell.h"

// Syscall numbers (must match kernel definitions)
#define SYSCALL_PRINT 1
#define SYSCALL_READ  2
#define SYSCALL_EXIT  3

extern void restore_cpu_state(registers_t* registers);


void user_print(const char* msg) {
    syscall(SYSCALL_PRINT, (uint64_t)msg, 0);
}

void user_read(char* buffer, size_t size) {
    syscall(SYSCALL_READ, (uint64_t)buffer, size);
}

void user_exit() {
    syscall(SYSCALL_EXIT, 0, 0);
}

void user_shell_main() {
    char buffer[128];

    user_print("Welcome to KeblaOS User Shell\n");
    while (1) {
        user_print("user@shell> ");
        user_read(buffer, sizeof(buffer));

        if (strcmp(buffer, "help") == 0) {
            user_print("Commands: help, echo, clear, exit\n");
        } else if (strncmp(buffer, "echo ", 5) == 0) {
            user_print(buffer + 5);
            user_print("\n");
        } else if (strcmp(buffer, "clear") == 0) {
            user_print("\033[2J\033[H");
        } else if (strcmp(buffer, "exit") == 0) {
            user_print("Exiting user shell.\n");
            user_exit(); // Terminates the process
        } else {
            user_print("Unknown command: ");
            user_print(buffer);
            user_print("\nType 'help' for a list of commands.\n");
        }
    }
}



void launch_user_shell() {
    process_t* shell_proc = create_process("UserShell");

    if (!shell_proc) {
        printf("User shell process creation failed!\n");
        return;
    }

    thread_t* shell_thread = create_thread(shell_proc, "UserShellThread", (void*)&user_shell_main, NULL);

    if (!shell_thread) {
        printf("User shell thread creation failed!\n");
        return;
    }

    restore_cpu_state((registers_t *)(uintptr_t)&shell_thread->registers);
}



