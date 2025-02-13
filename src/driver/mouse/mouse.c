#include <stdbool.h>
#include <stddef.h>

#include "../vga/framebuffer.h"
#include "../vga/vga_term.h"
#include "../vga/vga_gfx.h"
#include "../vga/color.h"

#include "../io/ports.h"

#include "mouse.h"



#define MOUSE_CMD_PORT 0x64
#define MOUSE_DATA_PORT 0x60

extern uint64_t fb_width;
extern uint64_t fb_height;

static int mouse_x = 40; // Initial cursor x position
static int mouse_y = 12; // Initial cursor y position
static uint8_t mouse_cycle = 0;
static int8_t mouse_bytes[3];
static bool mouse_initialized = false;

// Mouse cursor dimensions and color
#define CURSOR_WIDTH 8
#define CURSOR_HEIGHT 8
#define CURSOR_COLOR COLOR_WHITE // White
#define CURSOR_BG_COLOR COLOR_BLACK // Black (to erase old position)

// Basic mouse packet handling
void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(MOUSE_CMD_PORT) & 1) == 1)
                return;
        }
    } else {
        while (timeout--) {
            if ((inb(MOUSE_CMD_PORT) & 2) == 0)
                return;
        }
    }
}

void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0xD4);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, data);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

void mouse_install() {
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0xA8); // Enable auxiliary device (mouse)
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0x20); // Get Compaq status byte
    uint8_t status = mouse_read() | 2;
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0x60); // Set Compaq status byte
    mouse_write(status);
    mouse_write(0xF6); // Default settings
    mouse_read();      // Acknowledge
    mouse_write(0xF4); // Enable packet streaming
    mouse_read();      // Acknowledge
    mouse_initialized = true;
}

// Draws the mouse cursor at its current position
void draw_mouse_cursor() {
    fill_rectangle(mouse_x, mouse_y, CURSOR_WIDTH, CURSOR_HEIGHT, CURSOR_COLOR);
}

// Erases the mouse cursor at its old position
void erase_mouse_cursor(int old_x, int old_y) {
    fill_rectangle(old_x, old_y, CURSOR_WIDTH, CURSOR_HEIGHT, CURSOR_BG_COLOR);
}

// Handle mouse data packets
void mouse_handler() {
    static int old_x = 40, old_y = 12;

    // Read and process mouse packet data
    mouse_bytes[mouse_cycle++] = mouse_read();
    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        int dx = mouse_bytes[1];
        int dy = -mouse_bytes[2];

        // Update mouse position
        erase_mouse_cursor(old_x, old_y);
        mouse_x += dx;
        mouse_y += dy; // Y-axis is inverted for VGA

        // Clamp to screen boundaries
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > (fb_width - CURSOR_WIDTH)) mouse_x = (fb_width - CURSOR_WIDTH);
        if (mouse_y > (fb_height - CURSOR_HEIGHT)) mouse_y = (fb_height - CURSOR_HEIGHT);

        draw_mouse_cursor();

        old_x = mouse_x;
        old_y = mouse_y;
    }
}

// PIT handler for periodic updates (if needed for cursor animations)
void pit_handler() {
    // This can be used to periodically call `mouse_handler` or animate the cursor
}

// Initialize the PIT and mouse
void initialize_drivers() {
    vga_init();
    clear_screen();
    mouse_install();
    draw_mouse_cursor(); // Initial drawing of the cursor
}

// Main loop to process mouse input
void main_loop() {
    while (true) {
        // Typically this would be handled by interrupt handlers
        mouse_handler(); // Process mouse packets
        // Sleep or handle other tasks
    }
}







