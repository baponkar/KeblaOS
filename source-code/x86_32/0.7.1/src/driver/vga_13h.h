#pragma once

// vga_graphics.h
#ifndef VGA_GRAPHICS_H
#define VGA_GRAPHICS_H

#include "../stdlib/stdint.h"

#define VGA_WIDTH 320
#define VGA_HEIGHT 200
#define VGA_MEMORY ((uint8_t*) 0xA0000)  // VGA video memory location for mode 13h
#define VGA_TOTAL_PIXELS (VGA_WIDTH * VGA_HEIGHT)

// Text settings
#define FONT_WIDTH 8
#define FONT_HEIGHT 8
#define MAX_COLUMNS (VGA_WIDTH / FONT_WIDTH)
#define MAX_ROWS (VGA_HEIGHT / FONT_HEIGHT)

// Function prototypes
void set_vga_mode();
void set_pixel(uint16_t x, uint16_t y, uint8_t color);
void clear_screen(uint8_t color);

// Text rendering functions
void draw_char(uint16_t x, uint16_t y, char c, uint8_t color);
void draw_string(uint16_t x, uint16_t y, const char* str, uint8_t color);
void draw_string_at(uint16_t col, uint16_t row, const char* str, uint8_t color);
void clear_text_screen(uint8_t color);

static void set_sequencer();
static void set_crtc();
static void set_graphics_controller();
static void set_attribute_controller();
static void disable_display();
static void enable_display();
static void set_palette();


#endif // VGA_GRAPHICS_H



