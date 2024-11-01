#pragma once
#include "../idt/idt.h"




void keyboardHandler(registers_t *regs);
int getScanCode();
char scanCodeToChar(uint32_t scanCode);
uint32_t char_to_scancode(char input);

void initKeyboard();



