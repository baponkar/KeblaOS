#pragma once
#include "../stdlib/stdint.h"
#include "../stdlib/string.h"
#include "ports.h"
#include "vga.h"
#include "../idt/idt.h"
#include "../usr/shell.h"


void keyboardHandler(registers_t *regs);
int getScanCode();
char scanCodeToChar(uint32_t scanCode);
uint32_t char_to_scancode(char input);
void read_input(char *command, int MAX_COMMAND_LEN);

void initKeyboard();



