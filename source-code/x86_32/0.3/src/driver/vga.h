
#pragma once

#include "../stdlib/stdint.h"


#define VIDEO_MEMORY_ADDRESS 0xB8000
#define WIDTH 80
#define HEIGHT 25

// colors
#define COLOR8_BLACK 0
#define COLOR8_BLUE 1
#define COLOR8_GREEN 2
#define COLOR8_CYAN 3
#define COLOR8_RED 4
#define COLOR8_MAGENTA 5
#define COLOR8_BROWN 6
#define COLOR8_LIGHT_GREY 7
#define COLOR8_DARK_GREY 8
#define COLOR8_LIGHT_BLUE 9
#define COLOR8_LIGHT_GREEN 10
#define COLOR8_LIGHT_CYAN 11
#define COLOR8_LIGHT_RED 12
#define COLOR8_LIGHT_MAGENTA 13
#define COLOR8_LIGHT_BROWN 14
#define COLOR8_WHITE 15



void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void set_cursor_pos(int x, int y);
void move_cursor();
uint16_t get_cursor_position(void);
int get_cursor_position_x(void);
int get_cursor_position_y(void);


void vga_clear();
void create_newline();
void putchar(unsigned char c);
void print(char* s);

void putchar_at(char c, int x, int y);
void print_at(char* s, int x, int y);

void print_hex(uint32_t n);
void print_dec(uint32_t n);



