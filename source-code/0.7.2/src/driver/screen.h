#ifndef SCREEN_H
#define SCREEN_H

#include "../stdlib/stdint.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200
#define VIDEO_ADDRESS 0xA0000

// Color Definitions (0-255)
#define BLACK      0
#define BLUE       1
#define GREEN      2
#define CYAN       3
#define RED        4
#define MAGENTA    5
#define BROWN      6
#define LIGHT_GRAY 7
#define DARK_GRAY  8
#define LIGHT_BLUE 9
#define LIGHT_GREEN 10
#define LIGHT_CYAN 11
#define LIGHT_RED 12
#define LIGHT_MAGENTA 13
#define YELLOW     14
#define WHITE      15
// ... define up to 255 as needed

// Function Prototypes
void set_video_mode_13h();
void set_palette();
void putpixel(int x, int y, uint8_t color);
void draw_line(int x1, int y1, int x2, int y2, uint8_t color);
void draw_rect(int x1, int y1, int x2, int y2, uint8_t color);
void fill_rect(int x1, int y1, int x2, int y2, uint8_t color);
void draw_circle(int xc, int yc, int radius, uint8_t color);
void draw_char(char ch, int x, int y, uint8_t color);
void print_string(const char *str, int x, int y, uint8_t color);
void clear_back_buffer();
void swap_buffers();

#endif // SCREEN_H

