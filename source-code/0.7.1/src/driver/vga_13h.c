// vga_graphics.c
#include "vga_13h.h"
#include "font.h"  // Include the font data
#include "../stdlib/stdint.h"
#include "../driver/ports.h"



// Set VGA mode 13h (320x200, 256 colors)
void set_vga_mode() {
    // Typically, mode setting is done via BIOS interrupts, but since we're in protected mode,
    // we'll use the VESA BIOS Extensions (VBE) or directly manipulate VGA registers.
    // For simplicity, this example assumes mode 13h can be set via I/O ports.

    // Example: Setting mode 13h using BIOS interrupt is not directly possible in protected mode.
    // You would need to use a VBE library or switch to real mode temporarily.
    // Here, we assume the mode is already set or handled elsewhere.

    disable_display();

    set_sequencer();
    set_crtc();
    set_graphics_controller();
    set_attribute_controller();

    set_palette(); // Optional: Initialize the color palette

    enable_display();

    // Clear video memory to black
    clear_screen(0x00);  // 0x00 is the color code for black
}

// Plot a pixel at (x, y) with a given color
void set_pixel(uint16_t x, uint16_t y, uint8_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return;  // Prevent writing out of bounds
    }
    uint32_t offset = y * VGA_WIDTH + x;
    VGA_MEMORY[offset] = color;
}

// Clear the screen with a single color
void clear_screen(uint8_t color) {
    for (uint32_t i = 0; i < VGA_TOTAL_PIXELS; i++) {
        VGA_MEMORY[i] = color;
    }
}

// Draw a single character at (x, y) with a given color
void draw_char(uint16_t x, uint16_t y, char c, uint8_t color) {
    if (c < 32 || c > 127) {
        c = '?';  // Replace unsupported characters with '?'
    }
    const uint8_t* char_bitmap = font[c - 32];
    for (uint8_t row = 0; row < FONT_HEIGHT; row++) {
        uint8_t row_bits = char_bitmap[row];
        for (uint8_t col = 0; col < FONT_WIDTH; col++) {
            if (row_bits & (1 << (7 - col))) {  // Most significant bit on the left
                set_pixel(x + col, y + row, color);
            }
        }
    }
}

// Draw a string starting at (x, y) with a given color
void draw_string(uint16_t x, uint16_t y, const char* str, uint8_t color) {
    uint16_t orig_x = x;
    while (*str) {
        if (*str == '\n') {
            y += FONT_HEIGHT;
            x = orig_x;
        } else {
            draw_char(x, y, *str, color);
            x += FONT_WIDTH;
            if (x + FONT_WIDTH > VGA_WIDTH) {  // Wrap to next line if exceeding width
                y += FONT_HEIGHT;
                x = orig_x;
            }
        }
        str++;
        if (y + FONT_HEIGHT > VGA_HEIGHT) {  // Stop if exceeding height
            break;
        }
    }
}

// Draw a string at a specific column and row with a given color
void draw_string_at(uint16_t col, uint16_t row, const char* str, uint8_t color) {
    if (col >= MAX_COLUMNS || row >= MAX_ROWS) {
        return;  // Out of bounds
    }
    uint16_t x = col * FONT_WIDTH;
    uint16_t y = row * FONT_HEIGHT;
    draw_string(x, y, str, color);
}

// Clear the text area (optional: clear the entire screen or specific regions)
void clear_text_screen(uint8_t color) {
    clear_screen(color);
    // Optionally, reset cursor position if you implement one
}




// VGA Register Ports
#define VGA_SEQ_INDEX     0x3C4
#define VGA_SEQ_DATA      0x3C5
#define VGA_CRTC_INDEX    0x3D4
#define VGA_CRTC_DATA     0x3D5
#define VGA_GC_INDEX      0x3CE
#define VGA_GC_DATA       0x3CF
#define VGA_AC_INDEX      0x3C0
#define VGA_AC_WRITE      0x3C0
#define VGA_AC_READ       0x3C1

// Sequencer Registers for Mode 13h
static const uint8_t seq_regs[5] = {
    0x03, // Reset
    0x01, // Clocking Mode
    0x0F, // Map Mask
    0x00, // Character Map Select
    0x0E  // Memory Mode
};

// CRTC Registers for Mode 13h
static const uint8_t crtc_regs[25] = {
    0x5F, // Horizontal Total
    0x4F, // Horizontal Display End
    0x50, // Start Horizontal Blank
    0x82, // End Horizontal Blank
    0x54, // Start Horizontal Retrace
    0x0B, // End Horizontal Retrace
    0x1F, // Vertical Total
    0x00, // Overflow
    0x4F, // Preset Row Scan
    0x0D, // Maximum Scan Line
    0x1F, // Cursor Start
    0x00, // Cursor End
    0x00, // Start Address High
    0x00, // Start Address Low
    0x00, // Cursor Location High
    0xE3, // Cursor Location Low
    0xFF, // Vertical Retrace Start
    0x0C, // Vertical Retrace End
    0xDF, // Vertical Display End
    0x28, // Offset
    0x1F, // Underline Location
    0x96, // Start Vertical Blank
    0xB9, // End Vertical Blank
    0xA3, // Mode Control
    0xFF  // Line Compare
};

// Graphics Controller Registers for Mode 13h
static const uint8_t gc_regs[9] = {
    0x00, // Set/Reset
    0x00, // Enable Set/Reset
    0x00, // Color Compare
    0x00, // Data Rotate
    0x00, // Read Map Select
    0x40, // Graphics Mode
    0x05, // Miscellaneous
    0x0F, // Color Don't Care
    0xFF  // Bit Mask
};

static void set_sequencer() {
    // Reset Sequencer
    outb(VGA_SEQ_INDEX, 0x00); // Reset register
    outb(VGA_SEQ_DATA, 0x03);  // Reset all

    // Wait for sequencer to reset
    while (inb(VGA_SEQ_DATA) & 0x01);

    // Write Sequencer registers
    for (int i = 0; i < 5; i++) {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, seq_regs[i]);
    }

    // Start sequencer
    outb(VGA_SEQ_INDEX, 0x00);
    outb(VGA_SEQ_DATA, 0x03);
}

static void set_crtc() {
    for(int i = 0; i < 25; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, crtc_regs[i]);
    }
}

static void set_graphics_controller() {
    for(int i = 0; i < 9; i++) {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, gc_regs[i]);
    }
}

static void set_attribute_controller() {
    // Unlock Attribute Controller
    outb(VGA_AC_INDEX, 0x20);

    // Set Attribute Controller registers
    // For mode 13h, basic setup suffices
    for(int i = 0; i < 21; i++) {
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, 0x00); // Modify as necessary
    }

    // Lock the Attribute Controller
    outb(VGA_AC_INDEX, 0x20);
}

static void disable_display() {
    // Disable display by setting bit 7 of CRTC register 0x17
    outb(VGA_CRTC_INDEX, 0x17);
    uint8_t crtc17 = inb(VGA_CRTC_DATA);
    outb(VGA_CRTC_DATA, crtc17 | 0x80); // Set bit 7 to disable display
}

static void enable_display() {
    // Re-enable display by clearing bit 7 of CRTC register 0x17
    outb(VGA_CRTC_INDEX, 0x17);
    uint8_t crtc17 = inb(VGA_CRTC_DATA);
    crtc17 &= ~0x80;
    outb(VGA_CRTC_DATA, crtc17);
}

static void set_palette() {
    outb(0x3C8, 0x00); // Start at palette index 0

    // Example: Set a grayscale palette
    for(int i = 0; i < 256; i++) {
        uint8_t color = i / 4; // 6-bit color (0-63)
        outb(0x3C9, color); // Red
        outb(0x3C9, color); // Green
        outb(0x3C9, color); // Blue
    }
}



