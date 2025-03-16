#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void vga_init();
void swap_buffers();


void set_pixel(int x, int y, uint32_t color);
void clear_screen();
void draw_char(int x, int y, char c, uint32_t color);
void scroll_up();
void update_cur_pos();
void draw_string(int x, int y, const char* str, uint32_t color);
void create_newline();
void backspace_manage();

void putchar(unsigned char c);
void print(const char* text);
void print_hex(uint64_t n);
void print_dec(uint64_t n);
void print_bin(uint64_t n);

size_t get_cursor_pos_x();
size_t get_cursor_pos_y();
void set_cursor_pos_x(size_t _pos_x);
void set_cursor_pos_y(size_t _pos_y);
void move_cur_up();
void move_cur_down();
void move_cur_left();
void move_cur_right();


void draw_checkmark(int x, int y, uint32_t color);

