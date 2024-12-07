#pragma once

#include "../driver/vga.h"  // Assuming you have a VGA text driver.
#include "../driver/ports.h"
#include "../stdlib/string.h"    // Assuming you have string manipulation functions.
#include "../stdlib/stdio.h"
#include "../driver/keyboard.h"  // Assuming you have keyboard input handling.
#include "../gdt/gdt.h"

#define BUFFER_SIZE 256

void shell_prompt();
void execute_command(char* command);
void shell();
void run_shell(bool is_shell_running);

void poweroff();
void reboot();
void print_registers_c(uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t esp, 
                       uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax);
extern void print_registers();
void print_features();
