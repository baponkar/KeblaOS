#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>




#define BUFFER_SIZE 256

void shell_prompt();
void execute_command(char* command);
void shell();
void run_shell(bool is_shell_running);
void print_features();


void shell_main();

