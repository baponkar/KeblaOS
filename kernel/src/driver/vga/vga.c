
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
            set_pixel(x, y, color);
        }
    }
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

// Display an image at position(x, y) in the framebuffer : ignoring black color
void draw_image_with_transparency(int x, int y,  const uint64_t *image_data, int img_width, int img_height) {
    for (int row = 0; row < img_height; row++) {
        for (int col = 0; col < img_width; col++) {
            uint32_t color = (uint32_t)image_data[row * img_width + col];   // take lower 32 bits
            if (color != 0x00000000) {                                      // skip transparent
                set_pixel(x + col, y + row, color);
            }
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

#define RAND_MAX 0x7FFFFFFF

// Milky Way galaxy drawing function
void draw_milkyway_galaxy(int center_x, int center_y, int scale) {
    // Galaxy parameters based on scale
    int bulge_radius = scale * 3;           // Central bulge
    int arm_count = 4;                      // Number of spiral arms
    int star_count = scale * 50;            // Number of stars
    int arm_length = scale * 15;            // Length of spiral arms
    
    // Galaxy colors
    uint32_t bulge_color = 0xFFFFA500;      // Orange-yellow for central bulge
    uint32_t arm_color = 0xFF4169E1;        // Royal blue for dense arm regions
    uint32_t star_color = 0xFFFFFFFF;       // White for stars
    uint32_t dust_color = 0xFF8B4513;       // Brown for dust lanes
    uint32_t halo_color = 0xFF483D8B;       // Dark blue for galactic halo
    
    // Draw galactic halo (faint background glow)
    for (int r = scale * 20; r > 0; r -= 5) {
        uint32_t color = halo_color & 0x00FFFFFF;  // Remove alpha
        color |= (0x10 + (r * 2)) << 24;           // Fade out with distance
        draw_circle(center_x, center_y, r, color);
    }
    
    // Draw central bulge with gradient
    for (int r = bulge_radius; r > 0; r--) {
        uint32_t color = bulge_color;
        // Make center brighter
        uint8_t alpha = 0x30 + (r * 0x10);
        color = (color & 0x00FFFFFF) | (alpha << 24);
        draw_circle(center_x, center_y, r, color);
    }
    
    // Draw spiral arms
    for (int arm = 0; arm < arm_count; arm++) {
        double angle_offset = (2 * 3.14159 * arm) / arm_count;
        
        for (int i = 0; i < arm_length; i++) {
            double t = i / (double)arm_length;
            double radius = 10 + t * (scale * 12);
            double angle = angle_offset + t * 10;  // Spiral angle
            
            // Spiral arm coordinates
            int arm_x = center_x + (int)(radius * cos(angle));
            int arm_y = center_y + (int)(radius * sin(angle));
            
            // Draw arm segment
            int arm_width = (int)((1 - t * 0.7) * 3);  // Arms get thinner
            draw_filled_circle(arm_x, arm_y, arm_width, arm_color);
            
            // Add dust lanes along arms
            if (i % 5 == 0) {
                int dust_x = center_x + (int)((radius - 2) * cos(angle + 0.1));
                int dust_y = center_y + (int)((radius - 2) * sin(angle + 0.1));
                draw_filled_circle(dust_x, dust_y, 2, dust_color);
            }
        }
    }
    
    // Draw stars with random distribution (more dense near center)
    for (int i = 0; i < star_count; i++) {
        // Generate polar coordinates
        double angle = ((double)rand() / RAND_MAX) * 2 * 3.14159;
        double distance = pow((double)rand() / RAND_MAX, 2) * scale * 18;
        
        // Convert to cartesian
        int star_x = center_x + (int)(distance * cos(angle));
        int star_y = center_y + (int)(distance * sin(angle));
        
        // Star size and brightness vary
        int star_size = 1 + (rand() % 3);
        uint8_t brightness = 0xC0 + (rand() % 0x3F);
        uint32_t color = (brightness << 16) | (brightness << 8) | brightness;
        
        // Add some colorful stars (blue giants, red dwarfs)
        if (rand() % 20 == 0) {
            if (rand() % 2) color = 0xFF4169E1;  // Blue
            else color = 0xFFFF4500;            // Red-orange
        }
        
        draw_filled_circle(star_x, star_y, star_size, color);
    }
    
    // Draw a few bright core stars
    for (int i = 0; i < 15; i++) {
        int offset_x = (rand() % (bulge_radius * 2)) - bulge_radius;
        int offset_y = (rand() % (bulge_radius * 2)) - bulge_radius;
        draw_filled_circle(center_x + offset_x, center_y + offset_y, 2, 0xFFFFFF00);
    }
    
    // Add some globular clusters in the halo
    for (int i = 0; i < 8; i++) {
        double angle = ((double)rand() / RAND_MAX) * 2 * 3.14159;
        double distance = scale * 15 + (rand() % (scale * 10));
        
        int cluster_x = center_x + (int)(distance * cos(angle));
        int cluster_y = center_y + (int)(distance * sin(angle));
        
        // Draw small cluster of stars
        for (int j = 0; j < 20; j++) {
            int offset_x = (rand() % 8) - 4;
            int offset_y = (rand() % 8) - 4;
            draw_filled_circle(cluster_x + offset_x, cluster_y + offset_y, 1, 0xFFF0F0F0);
        }
    }
}