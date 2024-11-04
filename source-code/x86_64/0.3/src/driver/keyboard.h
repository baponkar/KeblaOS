#pragma once
#include <stdint.h>
#include "../stdlib/string.h"
#include "ports.h"
#include "vga/vga.h"
#include "../idt/idt.h"
#include "../usr/shell.h"
#include "../util/util.h"
#include "../driver/speaker.h"


#define KEYBOARD_COMMAND_PORT 0x64  // Keyboard Command Port
#define KEYBOARD_DATA_PORT 0x60     // Keyboard Data Port

// Scan Code values
#define UNKNOWN    0xFFFFFFFF

#define ESC        0x00000001
#define NUM_1      0x00000002
#define NUM_2      0x00000003
#define NUM_3      0x00000004
#define NUM_4      0x00000005
#define NUM_5      0x00000006
#define NUM_6      0x00000007
#define NUM_7      0x00000008
#define NUM_8      0x00000009
#define NUM_9      0x0000000A
#define NUM_0      0x0000000B
#define MINUS      0x0000000C // '-'
#define EQUAL      0x0000000D // '='
#define BACKSPACE  0x0000000E
#define TAB        0x0000000F
#define Q          0x00000010
#define W          0x00000011
#define E          0x00000012
#define R          0x00000013
#define T          0x00000014
#define Y          0x00000015
#define U          0x00000016
#define I          0x00000017
#define O          0x00000018
#define P          0x00000019
#define LBRACKET   0x0000001A // '['
#define RBRACKET   0x0000001B // ']'
#define ENTER      0x0000001C
#define CTRL       0x0000001D
#define A          0x0000001E
#define S          0x0000001F
#define D          0x00000020
#define F          0x00000021
#define G          0x00000022
#define H          0x00000023
#define J          0x00000024
#define K          0x00000025
#define L          0x00000026
#define SEMICOLON  0x00000027 // ''
#define APOSTROPHE 0x00000028 // '\''
#define BACKTICK   0x00000029 // '`'
#define LSHIFT     0x0000002A
#define BACKSLASH  0x0000002B // '\'
#define Z          0x0000002C
#define X          0x0000002D
#define C          0x0000002E
#define V          0x0000002F
#define B          0x00000030
#define N          0x00000031
#define M          0x00000032
#define COMMA      0x00000033 // ','
#define PERIOD     0x00000034 // '.'
#define SLASH      0x00000035 // '/'
#define RSHIFT     0x00000036
#define NUMPAD_ASTERISK  0x00000037 // '*'
#define ALT       0x00000038
#define SPACE      0x00000039
#define CAPS_LOCK  0x0000003A

#define F1         0x0000003B
#define F2         0x0000003C
#define F3         0x0000003D
#define F4         0x0000003E
#define F5         0x0000003F
#define F6         0x00000040
#define F7         0x00000041
#define F8         0x00000042
#define F9         0x00000043
#define F10        0x00000044
#define NUM_LOCK   0x00000045
#define SCROLL_LOCK  0x00000046

#define NUMPAD_7   0x00000047
#define NUMPAD_8   0x00000048
#define NUMPAD_9   0x00000049
#define NUMPAD_MINUS  0x0000004A
#define NUMPAD_4   0x0000004B
#define NUMPAD_5   0x0000004C
#define NUMPAD_6   0x0000004D
#define NUMPAD_PLUS  0x0000004E
#define NUMPAD_1   0x0000004F
#define NUMPAD_2   0x00000050
#define NUMPAD_3   0x00000051
#define NUMPAD_0   0x00000052
#define NUMPAD_PERIOD  0x00000053 // '.'

#define F11        0x00000057
#define F12        0x00000058

#define HOME       0x00000047 // Same as NUMPAD_7 without Num Lock
#define UP         0x00000048 // Same as NUMPAD_8 without Num Lock
#define PAGE_UP    0x00000049 // Same as NUMPAD_9 without Num Lock
#define LEFT       0x0000004B // Same as NUMPAD_4 without Num Lock
#define RIGHT      0x0000004D // Same as NUMPAD_6 without Num Lock
#define END        0x0000004F // Same as NUMPAD_1 without Num Lock
#define DOWN       0x00000050 // Same as NUMPAD_2 without Num Lock
#define PAGE_DOWN  0x00000051 // Same as NUMPAD_3 without Num Lock
#define INSERT     0x00000052 // Same as NUMPAD_0 without Num Lock
#define DELETE     0x00000053 // Same as NUMPAD_PERIOD without Num Lock

int getScanCode();
bool getKeyState();
char scanCodeToChar(uint32_t scanCode);
void key_ctrl(uint32_t scanCode, bool keyPress);
void keyboardHandler(registers_t *regs);
void initKeyboard();
void disableKeyboard();

void handel_enter_key(bool keyPressed);
void handel_shift_key(bool keyPressed);
void handel_caps_lock_key(bool keyPressed);
void handel_backspace_key(bool keyPressed);
void handel_del_key(bool keyPressed);

void read_command(char* input);
bool is_printable(char ch);

