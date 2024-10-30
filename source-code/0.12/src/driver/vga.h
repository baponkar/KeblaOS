
#pragma once

#include "../stdlib/stdint.h"


#define VIDEO_MEMORY_ADDRESS 0xB8000


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


typedef struct {
    char character;
    uint8_t color;
} vga_cell_t;



void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void set_cursor_pos(int col, int row);
void move_cursor();
uint16_t get_cursor_position(void);
int get_cursor_position_col(void);
int get_cursor_position_row(void);

void move_cursor_up();
void move_cursor_down();
void move_cursor_left();
void move_cursor_right(int max_right_pos);

void cursor_offset(int row, int col);


void vga_clear();
void create_newline();
void scroll_up();

void backspace_manage();
void del_manage();

void putchar(unsigned char c);
void print(char* s);

void errprint(char *s);
void color_print(char *s, uint8_t back_color, uint8_t font_color);
void reset_color();

void putchar_at(char c, int col, int row);
void print_at(char* s, int col, int row);

void print_hex(uint32_t n);
void print_hex_at(uint32_t n, int col, int row);

void print_dec(uint32_t n);
void print_dec_at(uint32_t n, int col, int row);

void print_bin(uint32_t n);
void print_bin_at(uint32_t n, int col, int row);

void print_all_cell(char c);

vga_cell_t get_vga_cell(int row, int col);


void PANIC(char * s);