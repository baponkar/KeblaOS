#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../limine/limine.h"

#include "../stdlib/stdlib.h"

#include "../kernel/kernel.h"

#include "font.h"

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

#define DEFAULT_TEXT_COLOR 0xFFFFFFFF  // White
#define DEFAULT_BACK_COLOR 0xFF000000  // Black
#define DEFAULT_FONT_SIZE 16
#define DEFAULT_TEXT_LINE_GAP 4



void cls();




void draw_line( int x1, int y1, int x2, int y2, uint32_t color);


// Clear screen with a specific color
void cls_color( uint32_t color);

// Set a pixel at a specified (x, y) position with a color
void set_pixel(size_t x, size_t y, uint32_t color);

// Print text at a specific location with a font size and color
void print(const char* text);

// Draw vertical and horizontal lines
void draw_vertical_line(int x, int y, int length, uint32_t color);
void draw_horizontal_line(int x, int y, int length, uint32_t color);

// Draw basic shapes: rectangle, circle, triangle, and lines
void draw_rectangle(int x, int y, int width, int height, uint32_t color);
void draw_circle(int center_x, int center_y, int radius, uint32_t color);
void draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void fill_rectangle( int x, int y, int width, int height, uint32_t color);
void fill_circle( int center_x, int center_y, int radius, uint32_t color);
void draw_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
void fill_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);

// Draw a gradient background
void draw_gradient(uint32_t start_color, uint32_t end_color);


void draw_colorful_image();

void display_image(int x, int y, const uint32_t* image_data, int img_width, int img_height);


void update_cur_pos();
void create_newline();
void putchar(unsigned char c);

void print_hex(uint32_t n);
void print_dec(uint32_t n);
void print_bin(uint32_t n);