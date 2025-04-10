#pragma once

#include <stdint.h>
#include <stddef.h>

void draw_line( int x1, int y1, int x2, int y2, uint64_t color);
void draw_vertical_line( int x, int y, int length, uint64_t color);
void draw_horizontal_line( int x, int y, int length, uint64_t color);
void draw_rectangle( int x, int y, int width, int height, uint64_t color);
void draw_circle( int center_x, int center_y, int radius, uint64_t color);
void cls_color( uint64_t color);
void fill_rectangle( int x, int y, int width, int height, uint64_t color);
void fill_circle( int center_x, int center_y, int radius, uint64_t color);
void draw_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint64_t color);
void fill_flat_bottom_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint64_t color);
void fill_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint64_t color);
void draw_gradient( uint64_t start_color, uint64_t end_color);
void draw_colorful_image();
void display_image( int x, int y, const uint64_t* image_data, int img_width, int img_height);

void animation_demo();
void flower_bloom_animation(int center_x, int center_y);
void load_image_with_animation(int x, int y, const uint64_t* image_data, int img_width, int img_height);


