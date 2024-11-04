#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../../util/util.h"
#include "../../kernel/kernel.h"
#include "font.h"

#define VGA_INDEX_REG_PORT 0x3D4    //VGA CRT Controller Index Register
#define VGA_DATA_REG_PORT 0x3D5     //0x3D5: VGA CRT Controller Data Register


#define COLOR_BLACK       0xFF000000  // Black
#define COLOR_WHITE       0xFFFFFFFF  // White
#define COLOR_RED         0xFFFF0000  // Red
#define COLOR_GREEN       0xFF00FF00  // Green
#define COLOR_BLUE        0xFF0000FF  // Blue
#define COLOR_YELLOW      0xFFFFFF00  // Yellow
#define COLOR_CYAN        0xFF00FFFF  // Cyan
#define COLOR_MAGENTA     0xFFFF00FF  // Magenta
#define COLOR_SILVER      0xFFC0C0C0  // Silver
#define COLOR_GRAY        0xFF808080  // Gray
#define COLOR_MAROON      0xFF800000  // Maroon
#define COLOR_OLIVE       0xFF808000  // Olive
#define COLOR_LIME        0xFF00FF00  // Lime
#define COLOR_AQUA        0xFF00FFFF  // Aqua
#define COLOR_TEAL        0xFF008080  // Teal
#define COLOR_NAVY        0xFF000080  // Navy
#define COLOR_FUCHSIA     0xFFFF00FF  // Fuchsia
#define COLOR_PURPLE      0xFF800080  // Purple
#define COLOR_ORANGE      0xFFFFA500  // Orange
#define COLOR_BROWN       0xFFA52A2A  // Brown
#define COLOR_PINK        0xFFFFC0CB  // Pink
#define COLOR_GOLD        0xFFFFD700  // Gold
#define COLOR_LIGHT_GRAY  0xFFD3D3D3  // Light Gray
#define COLOR_DARK_GRAY   0xFFA9A9A9  // Dark Gray
#define PELLTE_BLUE_COLOR 0x000D92F4
#define PELLTE_LIGHT_BLUE_COLOR 0x0077CDFF
#define PELLTE_LIGHT_RED_COLOR 0x00F95454
#define PELLTE_RED_COLOR 0x00C62E2E

#define DEFAULT_TEXT_COLOR 0xFFFFFFFF  // White
#define DEFAULT_BACK_COLOR 0xFF000000  // Black

#define DEFAULT_FONT_SIZE 16
#define DEFAULT_TEXT_LINE_GAP 0

#define DEFAULT_FONT_WIDTH 8
#define DEFAULT_FONT_HEIGHT 16

void cls();
void set_pixel(size_t x, size_t y, uint32_t color);
void print_char_at(size_t line_no, size_t column_no, unsigned char c);
void update_line_col_no();
void create_newline();
void putchar(unsigned char c);
void print(const char* text);
void print_hex(uint64_t n);
void print_bin(uint64_t n);
void print_dec(uint64_t n);
void scroll_up();