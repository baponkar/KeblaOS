#pragma once

#include "../driver/vga.h"  // Assuming you have a VGA text driver.
#include "../driver/ports.h"
#include "../stdlib/string.h"    // Assuming you have string manipulation functions.
#include "../driver/keyboard.h"  // Assuming you have keyboard input handling.

#define BUFFER_SIZE 256

void execute_command(char* command);
void shell();
void poweroff();
void reboot();