
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BUFFER_SIZE 256       // For the complete command
#define KEYBOARD_BUF_SIZE 128 // Ring buffer capacity for keystrokes

void read_command(char *command, size_t bufsize);
void execute_command(char* command);
void run_kshell();
void kshell_main();

void start_kshell();

