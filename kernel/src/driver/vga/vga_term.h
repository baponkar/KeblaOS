#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void swap_buffers();


void clear_screen();
void draw_char(int x, int y, char c, uint32_t color);
void scroll_up();
void update_cur_pos();
void draw_string(int x, int y, const char* str, uint32_t color);
void create_newline();
void backspace_manage();

void putchar(unsigned char c);
void print(const char* text);


size_t get_cursor_pos_x();
size_t get_cursor_pos_y();
void set_cursor_pos_x(size_t _pos_x);
void set_cursor_pos_y(size_t _pos_y);
void move_cur_up();
void move_cur_down();
void move_cur_left();
void move_cur_right();


void draw_checkmark(int x, int y, uint32_t color);

void color_print(const char* text, uint32_t color);
// void toggle_cursor();
void draw_cursor();


