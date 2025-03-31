
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


void read_command(char *command, size_t bufsize);
void shell_prompt();
void execute_command(char* command);
void run_shell();
void shell_main();

void _start();
void start_shell();
