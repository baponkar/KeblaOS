#pragma once

#include "../stdlib/stdint.h"
#include "../stdlib/math.h"

#include "font.h"
#include "ports.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define COLOR(_r, _g, _b)                                                      \
    ((uint8_t)((((_r) & 0x7) << 5) | (((_g) & 0x7) << 2) | (((_b) & 0x3) << 0)))

#define BLACK COLOR(0, 0, 0)
#define WHITE COLOR(7, 7, 3)
#define RED COLOR(7, 0, 0)
#define GREEN COLOR(0, 7, 0)
#define BLUE COLOR(0, 0, 3)

#define YELLOW COLOR(7, 7, 0)
#define MAGENTA COLOR(7, 0, 3)
#define CYAN COLOR(0, 7, 3)

extern uint8_t back_buf[SCREEN_HEIGHT][SCREEN_WIDTH];

void screen_init(void);

void putpixel(int pos_x, int pos_y, uint8_t VGA_COLOR);

void fill_screen(uint8_t VGA_COLOR);

void draw_line(int x1, int y1, int x2, int y2, uint8_t VGA_COLOR);

void draw_rect(int x1, int y1, int x2, int y2, uint8_t VGA_COLOR);
void fill_rect(int x1, int y1, int x2, int y2, uint8_t VGA_COLOR);

void print_string(const char *str, int x, int y, uint8_t VGA_COLOR);

// double buffer stuff
void clear_buffer1(void);
void swap_buffers(void);

