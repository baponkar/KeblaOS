#pragma once
#include "../stdlib/stdint.h"
#include "../stdlib/string.h"
#include "ports.h"
#include "vga.h"
#include "../idt/idt.h"
#include "../usr/shell.h"
#include "../util/util.h"
#include "../driver/speaker.h"

int getScanCode();
bool getKeyState();
char scanCodeToChar(uint32_t scanCode);
void key_ctrl(uint32_t scanCode, bool keyPress);
void keyboardHandler(registers_t *regs);
void initKeyboard();

void handel_enter_key(bool keyPressed);
void handel_shift_key(bool keyPressed);
void handel_caps_lock_key(bool keyPressed);
void handel_backspace_key(bool keyPressed);
void handel_del_key(bool keyPressed);

void read_command(char* input);
bool is_printable(char ch);

