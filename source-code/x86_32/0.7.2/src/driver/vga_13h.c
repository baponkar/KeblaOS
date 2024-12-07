#include "vga_13h.h"

#define VIDEO_ADDRESS 0xA0000

// VGA control port addresses
#define PALETTE_MASK 0x3C6
#define PALETTE_READ 0x3C7
#define PALETTE_WRITE 0x3C8
#define PALETTE_DATA 0x3C9

uint8_t back_buf[SCREEN_HEIGHT][SCREEN_WIDTH];

void screen_init() {
    // configure palette with 8-bit RRRGGGBB color
    outb(PALETTE_MASK, 0xFF);
    outb(PALETTE_WRITE, 0);
    for (uint8_t i = 0; i < 255; i++) {
        outb(PALETTE_DATA, (((i >> 5) & 0x7) * (256 / 8)) / 4);
        outb(PALETTE_DATA, (((i >> 2) & 0x7) * (256 / 8)) / 4);
        outb(PALETTE_DATA, (((i >> 0) & 0x3) * (256 / 4)) / 4);
    }

    // set color 255 = white
    outb(PALETTE_DATA, 0x3F);
    outb(PALETTE_DATA, 0x3F);
    outb(PALETTE_DATA, 0x3F);
}

void putpixel(int pos_x, int pos_y, uint8_t VGA_COLOR) {
    if (pos_x >= 0 && pos_x < SCREEN_WIDTH && pos_y >= 0 &&
        pos_y < SCREEN_HEIGHT) {
        uint8_t *location = (uint8_t *)VIDEO_ADDRESS + SCREEN_WIDTH * pos_y + pos_x;
        *location = VGA_COLOR;
    }
}

void fill_screen(uint8_t VGA_COLOR) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            back_buf[y][x] = VGA_COLOR;
        }
    }
}

void draw_line(int x1, int y1, int x2, int y2, uint8_t VGA_COLOR) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    float xIncrement = dx / (float)steps;
    float yIncrement = dy / (float)steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= steps; i++) {
        back_buf[(int)y][(int)x] = VGA_COLOR;
        x += xIncrement;
        y += yIncrement;
    }
}

void draw_rect(int x1, int y1, int x2, int y2, uint8_t VGA_COLOR) {
    draw_line(x1, y1, x2, y1, VGA_COLOR);
    draw_line(x1, y2, x2, y2, VGA_COLOR);
    draw_line(x1, y1, x1, y2, VGA_COLOR);
    draw_line(x2, y1, x2, y2, VGA_COLOR);
}

void fill_rect(int x1, int y1, int x2, int y2, uint8_t VGA_COLOR) {
    for (int y = y1; y <= y2; y++) {
        draw_line(x1, y, x2, y, VGA_COLOR);
    }
}

static void draw_char(char ch, int x, int y, uint8_t VGA_COLOR) {
    for (int i = 0; i < FONT_SIZE; i++) {
        for (int j = 0; j < FONT_SIZE; j++) {
            back_buf[y + i][x + j] = (font[ch][i] & 1 << j) ? VGA_COLOR : BLACK;
        }
    }
}

void print_string(const char *str, int x, int y, uint8_t VGA_COLOR) {
    int offset = 0;
    while (*str) {
        char ch = *str++;
        if (ch == '\n') {
            y += FONT_SIZE;
            offset = 0;
        } else {
            draw_char(ch, x + offset, y, VGA_COLOR);
            offset += FONT_SIZE;
        }
    }
}

// double buffer stuff
void clear_buffer1(void) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            back_buf[y][x] = BLACK;
        }
    }
}

void swap_buffers(void) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            putpixel(x, y, back_buf[y][x]);
        }
    }
}
