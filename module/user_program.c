
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "libc/include/syscall.h"
#include "libc/include/stdio.h"
#include "libc/include/string.h"

#define MAX_INPUT 128

__attribute__((section(".text")))
void _start_1() {

    printf("user_rogram.elf\n");

    char input[MAX_INPUT];

    while (1) {
        printf("input> ");

        // Clear input buffer
        memset(input, 0, sizeof(input));

        // Read from keyboard
        syscall_keyboard_read((uint8_t*)input, MAX_INPUT);

        // Echo what was read
        printf("You typed: ");
        printf(input);
        printf("\n");

        if (strcmp(input, "exit") == 0) {
            printf("Exiting test program.\n");
            syscall_exit();
        }
    }

    while (true) {} // Halt
}
