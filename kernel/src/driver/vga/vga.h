#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../../../ext_lib/limine-9.2.3/limine.h"

extern uint32_t *fb0_address;
extern uint64_t fb0_width;
extern uint64_t fb0_height;
extern uint64_t fb0_pitch;
extern uint16_t fb0_bpp;
extern void *fb0_unused_ptr;
extern uint64_t fb0_edid_size;
extern void *fb0_edid;

void init_vga();

void set_pixel(int x, int y, uint32_t color);
uint32_t get_pixel(int x, int y);
void cls_color( uint32_t color);



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
void draw_image_with_transparency(int x, int y,  const uint64_t *image_data, int img_width, int img_height);

void load_image_with_animation(int x, int y, const uint64_t* image_data, int img_width, int img_height);


void draw_milkyway_galaxy(int center_x, int center_y, int scale);





