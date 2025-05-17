/*
PS/2 Mouse Driver for VGA with APIC support
This is a simple PS/2 mouse driver for VGA graphics mode.
It handles basic mouse movement and drawing a cursor on the screen.
// Found from : https://forum.osdev.org/viewtopic.php?t=10247
*/


#include "../../sys/cpu/cpu.h"
#include "../vga/framebuffer.h"
#include "../vga/vga_term.h"     // vga_init, print_bootloader_info, print_memory_map, display_image
#include "../vga/vga_gfx.h"
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

extern uint64_t fb_width;
extern uint64_t fb_height;


volatile int mouse_x = 40; // Initial cursor x position
volatile int mouse_y = 12; // Initial cursor y position
volatile bool mouse_left_pressed = false;


uint8_t mouse_cycle = 0;
int8_t mouse_bytes[3];
bool mouse_initialized = false;



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
    fill_rectangle(mouse_x, mouse_y, CURSOR_WIDTH, CURSOR_HEIGHT, color);
}

// Erases the mouse cursor at the given old position.
void erase_mouse_cursor(int old_x, int old_y) {
    fill_rectangle(old_x, old_y, CURSOR_WIDTH, CURSOR_HEIGHT, CURSOR_BG_COLOR);
}



// Process incoming mouse data packets.
void mouse_handler() {

    int old_x = mouse_x;
    int old_y = mouse_y;

    mouse_bytes[mouse_cycle++] = mouse_read();
    
    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        // printf("%d ", mouse_bytes[0]);
        int dx = mouse_bytes[1];
        int dy = -mouse_bytes[2];  // Y-axis is inverted in VGA

        // Update mouse position
        erase_mouse_cursor(old_x, old_y);
        mouse_x += dx;
        mouse_y += dy;

        // Clamp to screen boundaries
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > (int) fb_width) mouse_x = (int) fb_width;
        if (mouse_y > (int) fb_height) mouse_y = (int) fb_height;

        draw_mouse_cursor(mouse_x, mouse_y, CURSOR_BG_COLOR);
    }
}


// This is the interrupt handler called by the APIC.
// It processes the mouse packet and signals the end of interrupt.
void mouseHandler(registers_t *regs) {
    // printf("Mouse Interrupt: X=%d, Y=%d\n", mouse_x, mouse_y); // Debug output
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

    mouse_initialized = true;
}


void mouse_init() {
    asm volatile("cli");  // Disable interrupts
    
    uint32_t flag = IOAPIC_EDGE_TRIG | IOAPIC_HIGH_ACTIVE | IOAPIC_FIXED | IOAPIC_UNMASKED;
    ioapic_route_irq(MOUSE_IRQ, 0, MOUSE_VECTOR, flag );    // For bootstrap cpu
    
    mouse_install();
    enable_mouse();   // Enable mouse interrupts via APIC
    asm volatile("sti");  // Enable interrupts

    printf("[Info] Successfully MOUSE initialized.\n");
}



