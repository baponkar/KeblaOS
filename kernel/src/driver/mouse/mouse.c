/*
PS/2 Mouse Driver for VGA with APIC support
This is a simple PS/2 mouse driver for VGA graphics mode.
It handles basic mouse movement and drawing a cursor on the screen.
Found from : https://forum.osdev.org/viewtopic.php?t=10247
Ref: https://wiki.osdev.org/PS/2_Mouse
*/


#include "../../sys/cpu/cpu.h"
#include "../vga/framebuffer.h"
#include "../vga/vga.h"     // vga_init, print_bootloader_info, print_memory_map, display_image
#include "../vga/color.h"


#include "../../arch/interrupt/apic/apic.h"        // APIC interrupt support
#include "../../arch/interrupt/apic/ioapic.h"      // IOAPIC support
#include "../../arch/interrupt/apic/apic_interrupt.h"   // Interrupt handler installation
#include "../../arch/interrupt/irq_manage.h"

#include "../io/ports.h"

#include "../../lib/string.h"
#include "../../lib/stdio.h"

#include "mouse.h"

// Mouse cursor dimensions and colors
#define CURSOR_WIDTH 8
#define CURSOR_HEIGHT 8
#define CURSOR_COLOR COLOR_RED      // Red
#define CURSOR_BG_COLOR COLOR_BLACK // Black (to erase old position)

#define MOUSE_IRQ 12       // IRQ number for PS/2 mouse
#define MOUSE_VECTOR 44    // APIC interrupt vector for PS/2 mouse

#define MOUSE_CMD_PORT 0x64
#define MOUSE_DATA_PORT 0x60

extern uint64_t fb0_width;
extern uint64_t fb0_height;


volatile int mouse_x = 40; // Initial cursor x position
volatile int mouse_y = 12; // Initial cursor y position

volatile bool mouse_left_pressed = false;
volatile bool mouse_right_pressed = false;
volatile bool mouse_middle_pressed = false;


uint8_t mouse_cycle = 0;
uint8_t mouse_bytes[3];      // Signed byte
bool mouse_initialized = false;

// Buffer to store the background under the cursor
static uint32_t cursor_background[CURSOR_WIDTH * CURSOR_HEIGHT];

// Save the background pixels under the cursor position
void save_cursor_background(int x, int y) {
    for (int i = 0; i < CURSOR_HEIGHT; i++) {
        for (int j = 0; j < CURSOR_WIDTH; j++) {
            int px = x + j;
            int py = y + i;
            
            if (px >= 0 && px < (int)fb0_width && py >= 0 && py < (int)fb0_height) {
                cursor_background[i * CURSOR_WIDTH + j] = get_pixel(px, py);
            } else {
                cursor_background[i * CURSOR_WIDTH + j] = CURSOR_BG_COLOR;
            }
        }
    }
}

// Restore the background pixels at the cursor position
void restore_cursor_background(int x, int y) {
    for (int i = 0; i < CURSOR_HEIGHT; i++) {
        for (int j = 0; j < CURSOR_WIDTH; j++) {
            int px = x + j;
            int py = y + i;
            
            if (px >= 0 && px < (int)fb0_width && py >= 0 && py < (int)fb0_height) {
                set_pixel(px, py, cursor_background[i * CURSOR_WIDTH + j]);
            }
        }
    }
}

void mouse_wait(int type) {
    int timeout = 100000; // Timeout for waiting
    if (type == 0) {
        while (--timeout && (inb(MOUSE_CMD_PORT) & 1) == 0);
    } else {
        while (--timeout && (inb(MOUSE_CMD_PORT) & 2) == 0);
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




// Draws the mouse cursor at its current position.
void draw_mouse_cursor(int mouse_x, int mouse_y, uint32_t color) {
    // Save the current background before drawing the cursor
    save_cursor_background(mouse_x, mouse_y);
    
    // Draw the cursor (simple rectangle for now)
    for (int i = 0; i < CURSOR_HEIGHT; i++) {
        for (int j = 0; j < CURSOR_WIDTH; j++) {
            int px = mouse_x + j;
            int py = mouse_y + i;
            
            if (px >= 0 && px < (int)fb0_width && py >= 0 && py < (int)fb0_height) {
                set_pixel(px, py, color);
            }
        }
    }
}


// Erases the mouse cursor at the given old position.
void erase_mouse_cursor(int old_x, int old_y) {
    // Restore the background that was behind the cursor
    restore_cursor_background(old_x, old_y);
}



// Process incoming mouse data packets.
static void mouse_handler() {

    int old_x = mouse_x;
    int old_y = mouse_y;

    uint8_t data = mouse_read();
    
    // Sync check and packet parsing
    if (mouse_cycle == 0) {
        if (!(data & 0x08)) return; // Wait for valid first byte
        mouse_bytes[0] = data;
        mouse_cycle++;
    } else if (mouse_cycle == 1) {
        mouse_bytes[1] = data;
        mouse_cycle++;
    } else if (mouse_cycle == 2) {
        mouse_bytes[2] = data;
        mouse_cycle = 0;

        uint8_t status = mouse_bytes[0];
        // Correct sign extension for 9-bit two's complement
        int dx = (int)((status & 0x10) ? (mouse_bytes[1] | 0xFFFFFF00) : mouse_bytes[1]);
        int dy = (int)((status & 0x20) ? (mouse_bytes[2] | 0xFFFFFF00) : mouse_bytes[2]);

        // Check overflow bits
        if (status & 0x40 || status & 0x80) {
            return; // discard this packet
        }

        // Update mouse position
        erase_mouse_cursor(old_x, old_y);
        mouse_x += dx;
        mouse_y -= dy; // Invert Y axis to match screen coordinates

        // Clamp to screen boundaries
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x >= (int) fb0_width - CURSOR_WIDTH) 
            mouse_x = (int) fb0_width - CURSOR_WIDTH;
        if (mouse_y >= (int) fb0_height - CURSOR_HEIGHT) 
            mouse_y = (int) fb0_height - CURSOR_HEIGHT;

        draw_mouse_cursor(mouse_x, mouse_y, CURSOR_COLOR);
        mouse_left_pressed = (status & 0x01);
    }

    // Update button
    mouse_left_pressed = (mouse_bytes[0] & 0x01) ? true : false;
    mouse_right_pressed = (mouse_bytes[0] & 0x02) ? true : false;
    mouse_middle_pressed = (mouse_bytes[0] & 0x04) ? true : false;
}


// This is the interrupt handler called by the APIC.
// It processes the mouse packet and signals the end of interrupt.
void mouseHandler(registers_t *regs) {
    mouse_handler();
    apic_send_eoi();  // Signal end-of-interrupt for APIC
}

// Enable mouse interrupts using APIC (no PIC modifications needed)
void enable_mouse() {
    irq_install(MOUSE_IRQ, &mouseHandler);
}

// Disable mouse interrupts
void disable_mouse() {
    irq_uninstall(MOUSE_VECTOR);
}


// Install and initialize the PS/2 mouse device.
void mouse_install() {

    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0xA8);  // Enable auxiliary device (mouse)

    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0x20);  // Get Compaq status byte

    mouse_wait(0);
    uint8_t status = mouse_read() | 2;
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0x60);  // Set Compaq status byte

    mouse_wait(1);
    outb(MOUSE_DATA_PORT, status);

    mouse_write(0xF6);           // Set default settings
    mouse_read();                // Acknowledge
    
    mouse_write(0xF4);           // Enable packet streaming
    mouse_read();                // Acknowledge

    enable_mouse();             // Enable mouse interrupts via APIC

    mouse_initialized = true;
}


void mouse_init() {
    asm volatile("cli");  // Disable interrupts
    
    uint32_t flag = IOAPIC_EDGE_TRIG | IOAPIC_HIGH_ACTIVE | IOAPIC_FIXED | IOAPIC_UNMASKED;
    ioapic_route_irq(MOUSE_IRQ, 0, MOUSE_VECTOR, flag );    // For bootstrap cpu

    mouse_install();

    asm volatile("sti");    // Enable interrupts

    printf("[Info] Successfully MOUSE initialized.\n");
}



// Get mouse position and button states
int get_mouse_x() { return mouse_x; }
int get_mouse_y() { return mouse_y; }
bool is_left_button_pressed() { return mouse_left_pressed; }
bool is_right_button_pressed() { return mouse_right_pressed; }
bool is_middle_button_pressed() { return mouse_middle_pressed; }


// Check if mouse is within a rectangle
bool mouse_in_rect(int x, int y, int width, int height) {
    int mx = get_mouse_x();
    int my = get_mouse_y();
    return (mx >= x && mx <= x + width && my >= y && my <= y + height);
}

















