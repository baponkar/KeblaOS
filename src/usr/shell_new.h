#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void shell_prompt();
void execute_command(char* command);
void shell_main();
