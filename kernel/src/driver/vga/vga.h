#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../../../ext_lib/limine-9.2.3/limine.h"


void init_vga();

void set_pixel(int x, int y, uint32_t color);

void cls_color( uint32_t color);


typedef struct point{
    uint64_t x;
    uint64_t y;
}point_t;

point_t set_point(uint64_t x, uint64_t y);

void draw_line( int x1, int y1, int x2, int y2, uint32_t color);
void draw_vertical_line( int x, int y, int length, uint32_t color);
void draw_horizontal_line( int x, int y, int length, uint32_t color);

void draw_rectangle( int x, int y, int width, int height, uint32_t color);

void draw_circle( int center_x, int center_y, int radius, uint32_t color);
void draw_filled_circle( int center_x, int center_y, int radius, uint32_t color);

void draw_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
void draw_filled_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);

void draw_gradient( uint64_t start_color, uint64_t end_color);

void display_image( int x, int y, const uint64_t* image_data, int img_width, int img_height);
void load_image_with_animation(int x, int y, const uint64_t* image_data, int img_width, int img_height);


