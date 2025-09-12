
#include "framebuffer.h"
#include "color.h"

#include "../../lib/stdlib.h"
#include "../../lib/math.h"

#include "../../sys/timer/apic_timer.h"

#include "vga.h"

uint64_t fb_revision;
uint64_t fb_count;
struct limine_framebuffer **all_fb_ptr;
struct limine_framebuffer *fb0;

uint32_t *fb0_address;
uint64_t fb0_width;
uint64_t fb0_height;
uint64_t fb0_pitch;
uint16_t fb0_bpp;
void *fb0_unused_ptr;
uint64_t fb0_edid_size;
void *fb0_edid;


void init_vga(){
    fb_revision = get_fb_revision();
    fb_count = get_fb_count();
    all_fb_ptr = get_all_fb_ptr();
    fb0 = get_first_fb_ptr();

    fb0_address = (uint32_t *)(uintptr_t)get_fb0_address();
    fb0_width =  get_fb0_width();
    fb0_height = get_fb0_height();
    fb0_pitch = get_fb0_pitch();
    fb0_bpp = get_fb0_bpp();
    fb0_unused_ptr = get_fb0_unused_ptr();
    fb0_edid_size = get_fb0_edid_size();
    fb0_edid = get_fb0_edid();
}


void set_pixel(int x, int y, uint32_t color){
    if (x < 0 || x >= fb0_width || y < 0 || y >= fb0_height) return;
    uint32_t *pixel = (uint32_t*)((uintptr_t)fb0_address + (y * fb0_pitch) + (x * 4));
    *pixel = color;
}

uint32_t get_pixel(int x, int y){
    if (x < 0 || x >= fb0_width || y < 0 || y >= fb0_height) return 0;
    uint32_t *pixel = (uint32_t*)((uintptr_t)fb0_address + (y * fb0_pitch) + (x * 4));

    return *pixel;
}

void cls_color( uint32_t color) {
    for (size_t y = 0; y < fb0_height; y++) {
        for (size_t x = 0; x < fb0_width; x++) {
            // fb0_address[y * fb0_width + x] = color;
            set_pixel(x, y, color);
        }
    }
}

point_t set_point(uint64_t x, uint64_t y){
    point_t pt;
    pt.x = x;
    pt.y = y;

    return pt;
}


void draw_line( int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        set_pixel( x1, y1, color);  // Plot the current point

        // Check if we have reached the end point
        if (x1 == x2 && y1 == y2) {
            break;
        }

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void draw_vertical_line( int x, int y, int length, uint32_t color){
    // Drawing vertical line
    for(int i=y; i<y+length; i++){
        fb0_address[i * fb0_width + x] = color;
    }
}


void draw_horizontal_line( int x, int y, int length, uint32_t color){
    // Drawing horizontal line
    for(int i=x; i<x+length; i++){
        fb0_address[y * fb0_width + i] = color;
    }
}

void draw_rectangle( int x, int y, int width, int height, uint32_t color){
    draw_horizontal_line( x, y, width, color);
    draw_horizontal_line( x, y+height, width, color);

    draw_vertical_line( x, y, height, color);
    draw_vertical_line( x+width, y, height, color);
}

void draw_filled_rectangle( int x, int y, int width, int height, uint32_t color) {
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            set_pixel( j, i, color);
        }
    }
}

void draw_circle( int center_x, int center_y, int radius, uint32_t color) {
    int x = radius;
    int y = 0;
    int decision = 1 - x;  // Initial decision parameter

    while (x >= y) {
        // Plot the eight symmetrical points
        set_pixel( center_x + x, center_y + y, color);
        set_pixel( center_x - x, center_y + y, color);
        set_pixel( center_x + x, center_y - y, color);
        set_pixel( center_x - x, center_y - y, color);
        set_pixel( center_x + y, center_y + x, color);
        set_pixel( center_x - y, center_y + x, color);
        set_pixel( center_x + y, center_y - x, color);
        set_pixel( center_x - y, center_y - x, color);

        y++;

        if (decision <= 0) {
            decision += 2 * y + 1;
        } else {
            x--;
            decision += 2 * (y - x) + 1;
        }
    }
}

void draw_filled_circle( int center_x, int center_y, int radius, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                set_pixel( center_x + x, center_y + y, color);
            }
        }
    }
}


void draw_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
    draw_line( x1, y1, x2, y2, color);
    draw_line( x2, y2, x3, y3, color);
    draw_line( x3, y3, x1, y1, color);
}




void draw_gradient( uint64_t start_color, uint64_t end_color) {
    for (size_t y = 0; y < fb0_height; y++) {
        uint32_t color = start_color + (y * ((end_color - start_color) / fb0_height));
        for (size_t x = 0; x < fb0_width; x++) {
            set_pixel( x, y, color);
        }
    }
}

// Display an image at position (x, y) in the framebuffer
void display_image( int x, int y, const uint64_t* image_data, int img_width, int img_height) {
    for (int i = 0; i < img_height; i++) {
        for (int j = 0; j < img_width; j++) {
            uint32_t color = image_data[i * img_width + j];
            set_pixel(x + j, y + i, color);
        }
    }
}

void load_image_with_animation(int x, int y, const uint64_t* image_data, int img_width, int img_height) {
    cls_color(COLOR_BLACK ); // Clear screen first
    int bar_width = 10; // Width of the loading bar (pixels)

    cls_color(0xFF000000); // Clear screen to black first

    for (int offset = 0; offset <= img_width; offset += bar_width) {
        // Draw only part of the image (from left to right)
        for (int i = 0; i < img_height; i++) {
            for (int j = 0; j < offset && j < img_width; j++) {
                uint32_t color = image_data[i * img_width + j];
                draw_filled_rectangle(x + j, y + i, 1, 1, color);
            }
        }

        apic_delay(300); // Small delay to make the loading visible
    }

    // After animation is complete, show the full image cleanly
    display_image(x, y, image_data, img_width, img_height);
    apic_delay(1000); // Hold the image for a moment before clearing
    
    cls_color(COLOR_BLACK ); // Clear screen again
}



