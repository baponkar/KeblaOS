#include "screen.h"
#include "ports.h"
#include "../stdlib/math.h"
#include "font.h"


// Back Buffer
static uint8_t back_buf[SCREEN_HEIGHT][SCREEN_WIDTH];

// Font Data (8x8 font for each ASCII character)
//extern static constant uint8_t font[128][8]; // Define this array in a separate file (e.g., font.c)

// VGA Port Addresses
#define VGA_MISC_WRITE      0x3C2
#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define VGA_DAC_READ_INDEX  0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA        0x3C9
#define VGA_INSTAT_READ     0x3DA
#define VGA_CRTC_INDEX      0x3D4
#define VGA_CRTC_DATA       0x3D5

// Set VGA Mode 13h (320x200, 256 colors)
void set_video_mode_13h() {
    // Set the Miscellaneous Output Register
    outb(VGA_MISC_WRITE, 0x63); // Typical value for Mode 13h

    // Reset Sequencer
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x03); // Reset
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x01); // Clear Reset

    // Set Sequencer Registers
    outb(VGA_SEQ_INDEX, 0x01);
    outb(VGA_SEQ_DATA, 0x00); // Map Mask

    outb(VGA_SEQ_INDEX, 0x02);
    outb(VGA_SEQ_DATA, 0x0F); // Character Map Select

    outb(VGA_SEQ_INDEX, 0x03);
    outb(VGA_SEQ_DATA, 0x00); // Memory Mode

    outb(VGA_SEQ_INDEX, 0x04);
    outb(VGA_SEQ_DATA, 0x0E); // Extended Memory

    // Set Graphics Controller Registers
    outb(0x3CE, 0x00);
    outb(0x3CF, 0x00); // Set Reset

    outb(0x3CE, 0x01);
    outb(0x3CF, 0x00); // Enable Set/Reset

    outb(0x3CE, 0x02);
    outb(0x3CF, 0x05); // Set/Reset XOR

    outb(0x3CE, 0x03);
    outb(0x3CF, 0x00); // Graphics Mode

    outb(0x3CE, 0x04);
    outb(0x3CF, 0x00); // Color Don't Care

    outb(0x3CE, 0x05);
    outb(0x3CF, 0x0F); // Bit Mask

    // Unlock CRTC Registers
    outb(VGA_CRTC_INDEX, 0x11);
    uint8_t crt1 = inb(VGA_CRTC_DATA);
    crt1 &= ~0x80; // Unlock CRTC
    outb(VGA_CRTC_DATA, crt1);

    // Set CRTC Registers for Mode 13h
    uint8_t crtc_regs[25] = {
        0x5F, // Horizontal Total
        0x4F, // Horizontal Display End
        0x50, // Start Horizontal Blank
        0x82, // End Horizontal Blank
        0x55, // Start Horizontal Retrace
        0x81, // End Horizontal Retrace
        0xBF, // Vertical Total
        0x1F, // Overflow
        0x4F, // Preset Row Scan
        0x9C, // Maximum Scan Line
        0x0E, // Cursor Start
        0x00, // Cursor End
        0xFF, // Start Address High
        0x00, // Start Address Low
        0x00, // Cursor Location High
        0x00, // Cursor Location Low
        0x00, // Vertical Retrace Start
        0x00, // Vertical Retrace End
        0x00, // Vertical Display End
        0x00, // Offset
        0x00, // Underline Location
        0x00, // Start Vertical Blank
        0x00, // End Vertical Blank
        0x00, // Mode Control
        0x00, // Line Compare
        0x00  // Unused
    };

    for (int i = 0; i < 25; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, crtc_regs[i]);
    }

    // Re-lock CRTC Registers
    outb(VGA_CRTC_INDEX, 0x11);
    crt1 = inb(VGA_CRTC_DATA);
    crt1 |= 0x80; // Lock CRTC
    outb(VGA_CRTC_DATA, crt1);

    // Set the Palette
    set_palette();
}

// Initialize the VGA Palette
void set_palette() {
    outb(VGA_DAC_WRITE_INDEX, 0); // Start palette index at 0
    for (int i = 0; i < 256; i++) {
        uint8_t r = (i & 0xE0) >> 2; // 3 bits red
        uint8_t g = (i & 0x1C) << 1; // 3 bits green
        uint8_t b = (i & 0x03) << 4; // 2 bits blue

        // VGA DAC expects 6-bit values (0-63)
        outb(VGA_DAC_DATA, r);
        outb(VGA_DAC_DATA, g);
        outb(VGA_DAC_DATA, b);
    }

    // Optionally, set specific colors
    // Example: Set MAGENTA (index 5) to full magenta
    outb(VGA_DAC_WRITE_INDEX, MAGENTA);
    outb(VGA_DAC_DATA, 63); // R (max 63)
    outb(VGA_DAC_DATA, 0);  // G
    outb(VGA_DAC_DATA, 63); // B (max 63)
}

// Put a pixel on the back buffer
void putpixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return; // Bounds check

    back_buf[y][x] = color;
}

// Swap the back buffer to the VGA memory
void swap_buffers() {
    uint8_t *video = (uint8_t *) VIDEO_ADDRESS;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            video[y * SCREEN_WIDTH + x] = back_buf[y][x];
        }
    }
}

// Clear the back buffer
void clear_back_buffer() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            back_buf[y][x] = BLACK;
        }
    }
}

// Bresenham's Line Algorithm
void draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        putpixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Draw a rectangle outline
void draw_rect(int x1, int y1, int x2, int y2, uint8_t color) {
    draw_line(x1, y1, x2, y1, color); // Top
    draw_line(x1, y2, x2, y2, color); // Bottom
    draw_line(x1, y1, x1, y2, color); // Left
    draw_line(x2, y1, x2, y2, color); // Right
}

// Fill a rectangle
void fill_rect(int x1, int y1, int x2, int y2, uint8_t color) {
    for (int y = y1; y <= y2; y++) {
        draw_line(x1, y, x2, y, color);
    }
}

// Midpoint Circle Algorithm
void draw_circle(int xc, int yc, int radius, uint8_t color) {
    int x = 0;
    int y = radius;
    int d = 1 - radius;

    while (x <= y) {
        putpixel(xc + x, yc + y, color);
        putpixel(xc - x, yc + y, color);
        putpixel(xc + x, yc - y, color);
        putpixel(xc - x, yc - y, color);
        putpixel(xc + y, yc + x, color);
        putpixel(xc - y, yc + x, color);
        putpixel(xc + y, yc - x, color);
        putpixel(xc - y, yc - x, color);

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

// Draw a character using an 8x8 font
void draw_char(char ch, int x, int y, uint8_t color) {
    if (ch < 0 || ch > 255)
        return; // Only support 0-255

    for (int row = 0; row < 8; row++) {
        uint8_t row_data = font[(unsigned char) ch][row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (1 << (7 - col))) {
                putpixel(x + col, y + row, color);
            }
        }
    }
}

// Print a string starting at (x, y)
void print_string(const char *str, int x, int y, uint8_t color) {
    while (*str) {
        if (*str == '\n') {
            y += 8; // Move to next line
            x = 0;  // Reset x to start
        } else {
            draw_char(*str, x, y, color);
            x += 8; // Move right by font width
        }
        str++;
    }
}


