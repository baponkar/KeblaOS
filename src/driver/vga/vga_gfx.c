/*

*/

#include "color.h"
#include "framebuffer.h"
#include "vga_settings.h"
#include "vga_term.h"

#include "../../lib/stdlib.h"
#include "../../lib/math.h"
#include "../../lib/stdio.h"
#include "../../lib/stdlib.h"


#include "../../x86_64/timer/apic_timer.h"

#include "vga_gfx.h"


void draw_line( int x1, int y1, int x2, int y2, uint64_t color) {
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


void draw_vertical_line( int x, int y, int length, uint64_t color){
    // Drawing vertical line
    for(int i=y; i<y+length; i++){
        fb_address[i * fb_width + x] = color;
    }
}


void draw_horizontal_line( int x, int y, int length, uint64_t color){
    // Drawing horizontal line
    for(int i=x; i<x+length; i++){
        fb_address[y * fb_width + i] = color;
    }
}


void draw_rectangle( int x, int y, int width, int height, uint64_t color){
    draw_horizontal_line( x, y, width, color);
    draw_horizontal_line( x, y+height, width, color);

    draw_vertical_line( x, y, height, color);
    draw_vertical_line( x+width, y, height, color);
}


void draw_circle( int center_x, int center_y, int radius, uint64_t color) {
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


void cls_color( uint64_t color) {
    for (size_t y = 0; y < fb_height; y++) {
        for (size_t x = 0; x < fb_width; x++) {
            fb_address[y * fb_width + x] = color;
        }
    }
}


void fill_rectangle( int x, int y, int width, int height, uint64_t color) {
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            set_pixel( j, i, color);
        }
    }
}


void fill_circle( int center_x, int center_y, int radius, uint64_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                set_pixel( center_x + x, center_y + y, color);
            }
        }
    }
}


void draw_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint64_t color) {
    draw_line( x1, y1, x2, y2, color);
    draw_line( x2, y2, x3, y3, color);
    draw_line( x3, y3, x1, y1, color);
}


void fill_flat_bottom_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint64_t color) {
    int inv_slope1 = (x2 - x1) / (y2 - y1);
    int inv_slope2 = (x3 - x1) / (y3 - y1);

    int curx1 = x1;
    int curx2 = x1;

    for (int scanlineY = y1; scanlineY <= y2; scanlineY++) {
        draw_horizontal_line( (int)curx1, scanlineY, (int)(curx2 - curx1), color);
        curx1 += inv_slope1;
        curx2 += inv_slope2;
    }
}


void fill_triangle( int x1, int y1, int x2, int y2, int x3, int y3, uint64_t color) {
    if (y2 < y1) { int temp_x = x1; x1 = x2; x2 = temp_x; int temp_y = y1; y1 = y2; y2 = temp_y; }
    if (y3 < y1) { int temp_x = x1; x1 = x3; x3 = temp_x; int temp_y = y1; y1 = y3; y3 = temp_y; }
    if (y3 < y2) { int temp_x = x2; x2 = x3; x3 = temp_x; int temp_y = y2; y2 = y3; y3 = temp_y; }

    if (y2 == y3) {
        fill_flat_bottom_triangle( x1, y1, x2, y2, x3, y3, color);
    } else if (y1 == y2) {
        fill_flat_bottom_triangle( x3, y3, x1, y1, x2, y2, color);
    } else {
        int x4 = x1 + (int)(y2 - y1) / (y3 - y1) * (x3 - x1);
        int y4 = y2;
        fill_flat_bottom_triangle( x1, y1, x2, y2, x4, y4, color);
        fill_flat_bottom_triangle( x3, y3, x2, y2, x4, y4, color);
    }
}


void draw_gradient( uint64_t start_color, uint64_t end_color) {
    for (size_t y = 0; y < fb_height; y++) {
        uint64_t color = start_color + (y * ((end_color - start_color) / fb_height));
        for (size_t x = 0; x < fb_width; x++) {
            set_pixel( x, y, color);
        }
    }
}


// Function to draw a simple colorful image
void draw_colorful_image() {
    // Draw gradient background
    draw_gradient( COLOR_BLUE, COLOR_CYAN);

    // Draw a central circle
    fill_circle( fb_width / 2, fb_height / 2, 100, COLOR_YELLOW);

    // Draw a border rectangle around the circle
    fill_rectangle( fb_width / 2 - 120, fb_height / 2 - 120, 240, 20, COLOR_MAGENTA);
    fill_rectangle( fb_width / 2 - 120, fb_height / 2 + 100, 240, 20, COLOR_MAGENTA);
    fill_rectangle( fb_width / 2 - 120, fb_height / 2 - 100, 20, 200, COLOR_MAGENTA);
    fill_rectangle( fb_width / 2 + 100, fb_height / 2 - 100, 20, 200, COLOR_MAGENTA);

    // Draw some additional colorful circles
    fill_circle( fb_width / 4, fb_height / 4, 50, COLOR_GREEN);
    fill_circle( 3 * fb_width / 4, fb_height / 4, 50, COLOR_RED);
    fill_circle( fb_width / 4, 3 * fb_height / 4, 50, COLOR_WHITE);
    fill_circle( 3 * fb_width / 4, 3 * fb_height / 4, 50, COLOR_BLUE);
}


// Display an image at position (x, y) in the framebuffer
void display_image( int x, int y, const uint64_t* image_data, int img_width, int img_height) {
    for (int i = 0; i < img_height; i++) {
        for (int j = 0; j < img_width; j++) {
            uint64_t color = image_data[i * img_width + j];
            set_pixel(x + j, y + i, color);
        }
    }
}


void animation_demo() {
    cls_color(0x000000); // Clear screen to black

    int center_x = 400, center_y = 300; // screen center (assuming 800x600 resolution)
    int radius = 10;
    int growing = 1;

    int rect_x = 0;
    int rect_y = 500;
    int rect_width = 80;
    int rect_height = 40;

    uint64_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00}; // red, green, blue, yellow
    int color_index = 0;
    
    for (int frame = 0; frame < 500; frame++) {
        // Clear the screen
        cls_color(0x000000);

        // 1. Draw a growing and shrinking circle at center
        fill_circle(center_x, center_y, radius, colors[color_index]);

        if (growing) {
            radius += 2;
            if (radius >= 80) {
                growing = 0;
                color_index = (color_index + 1) % 4;
            }
        } else {
            radius -= 2;
            if (radius <= 10) {
                growing = 1;
                color_index = (color_index + 1) % 4;
            }
        }

        // 2. Draw a moving rectangle horizontally
        fill_rectangle(rect_x, rect_y, rect_width, rect_height, 0x00FFFF); // Cyan rectangle
        rect_x += 5;
        if (rect_x > 800) rect_x = -rect_width; // Wrap around when off screen

        // 3. Draw a bouncing line
        int line_y = (frame % 600);
        draw_line(0, line_y, 800, 600 - line_y, 0xFF00FF); // Pink line

        // 4. Small spinning triangle around center
        int angle = frame % 360;
        int x1 = center_x + 50 * cos(angle * 3.1415 / 180);
        int y1 = center_y + 50 * sin(angle * 3.1415 / 180);
        int x2 = center_x + 50 * cos((angle + 120) * 3.1415 / 180);
        int y2 = center_y + 50 * sin((angle + 120) * 3.1415 / 180);
        int x3 = center_x + 50 * cos((angle + 240) * 3.1415 / 180);
        int y3 = center_y + 50 * sin((angle + 240) * 3.1415 / 180);
        fill_triangle(x1, y1, x2, y2, x3, y3, 0xFFFFFF); // White triangle

        // Short delay between frames
        apic_delay(16); // ~16 milliseconds for ~60 FPS
    }
}


void flower_bloom_animation(int center_x, int center_y) {
    int max_radius = 100; // Maximum size of flower
    int petals = 12;      // Number of petals
    uint64_t petal_color = 0xFFFF00FF; // Pinkish
    uint64_t center_color = 0xFFFFFF00; // Yellow
    uint64_t background_color = 0xFF000000; // Black background

    cls_color(background_color); // Clear screen first

    for (int r = 0; r <= max_radius; r += 2) { // Slowly grow
        cls_color(background_color); // Clear screen each frame

        // Draw petals
        for (int i = 0; i < petals; i++) {
            double angle_deg = (360.0 / petals) * i;
            double angle_rad = to_radians(angle_deg);

            int petal_center_x = center_x + (int)(r * 0.5 * cos(angle_rad));
            int petal_center_y = center_y + (int)(r * 0.5 * sin(angle_rad));

            fill_circle(petal_center_x, petal_center_y, r / 4, petal_color);
        }

        // Draw flower center
        fill_circle(center_x, center_y, r / 6, center_color);

        apic_delay(50); // Small delay
    }

    // After bloom complete, hold final image
    apic_delay(1000);
}


void load_image_with_animation(int x, int y, const uint64_t* image_data, int img_width, int img_height) {
    clear_screen(); // Clear screen first
    int bar_width = 10; // Width of the loading bar (pixels)

    cls_color(0xFF000000); // Clear screen to black first

    for (int offset = 0; offset <= img_width; offset += bar_width) {
        // Draw only part of the image (from left to right)
        for (int i = 0; i < img_height; i++) {
            for (int j = 0; j < offset && j < img_width; j++) {
                uint64_t color = image_data[i * img_width + j];
                fill_rectangle(x + j, y + i, 1, 1, color);
            }
        }

        apic_delay(300); // Small delay to make the loading visible
    }

    // After animation is complete, show the full image cleanly
    display_image(x, y, image_data, img_width, img_height);
    apic_delay(2500); // Hold the image for a moment before clearing
    
    clear_screen(); // Clear screen again
}



