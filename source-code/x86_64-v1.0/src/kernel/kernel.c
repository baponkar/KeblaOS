#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../util/util.h"
#include "../../limine/limine.h"
#include "../driver/vga.h"
#include "../gdt/gdt.h"
#include "../idt/idt.h"

uint32_t *fb_ptr = NULL;
size_t SCREEN_WIDTH;
size_t SCREEN_HEIGHT;

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.



// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}





gdt_entry_t test;

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    // Ensure the framebuffer is initialized
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    fb_ptr = (uint32_t* ) framebuffer->address;
    SCREEN_WIDTH = framebuffer->width;   // 1024
    SCREEN_HEIGHT = framebuffer->height; // 768

    cls();

    init_gdt();
    
    print_dec(sizeof(test)); // 16

    // test1.lower_half = (uint16_t) 0xFFFF;
    // test1.mid = 0xF;
    // test1.upper_half = (uint16_t) 0xFFFF;

    // print_hex(test1.upper_half);
    // create_newline();
    // print_hex(test1.mid);
    // create_newline();
    // print_hex(test1.lower_half);
    // init_idt();

    // print("Hello World");
    // create_newline();
    // print("Hello World");
    // create_newline();
    // print_hex(0x2f);
    // create_newline();
    // print_dec(34);
    // create_newline();
    // print_bin(0b110011);
    // create_newline();
    //check_gdt();
    // test_interrupt();

    // Example to print "HELLO" using the 8x8 font
    //print_text(fb_ptr, width, 50, 20, "HELLO", COLOR_WHITE, 8);


    // Example to print "WORLD" using the 8x16 font
    // Columun support 1023 max
    // Row support 245 max
    //print_text(fb_ptr, width, 50, 80, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-=;<>?~`,./!@#$%^&*()_+", COLOR_WHITE, 16);

    // set_pixel(fb_ptr, 10, 50, 50, COLOR_WHITE);
    // draw_vertical_line(fb_ptr, 50, 50, 100, COLOR_WHITE);

//     int y = 50;
//     int x = 50;
//     int rect_width = 100;
//     int rect_height = 80;
//     uint32_t color = COLOR_WHITE;

//     // draw_rectangle(fb_ptr, x, y, rect_width, rect_height, color);
//     draw_rectangle(x, y, 100, 100, color);
//     draw_rectangle( x+10, y+10, 110, 110, color);

//     draw_line( x,y, x+10,y+10,color);
//     draw_line( x+rect_width,y, x+rect_width+10,y+10,color);

//     draw_line( x,y+rect_height, x+10,y+rect_height+10,color);
//     draw_line( x+rect_width, y+rect_height, x+rect_width+10, y+rect_height+10, color);
//     draw_circle(512, 350, 100, color);
//     draw_triangle(100,100, 50,100, 75, 50, COLOR_RED);

//     fill_rectangle( 90,120, 40,40,COLOR_RED);
//     fill_circle( 400, 300, 100, COLOR_BLUE);
//     //fill_rectangle(fb_ptr, 100,100, 50,100, 75, 50, COLOR_RED);


//     draw_colorful_image();

//     cls();
//     // Example image data (small 3x3 image for demonstration)
//    const uint32_t small_image[24] = {
//         0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,  // Top row (white background)
//         0xFFFFFFFF, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,  // Eyes (black)
//         0xFFFFFFFF, 0xFF000000, 0xFF00FF00, 0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF,  // Mouth (green)
//         0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF   // Bottom row (white background)
//     };


    

//     cls();
//     fill_rectangle(0,700,1024,68,COLOR_BLUE);
//     print_text(20, 720,"KeblaOS", COLOR_WHITE,16);

//     draw_rectangle(40, 40, 950, 600, COLOR_WHITE);

//     print_text(50, 50, "Limine Bootloader", COLOR_WHITE, 16);
//     print_text(50, 70, "GDT Enabaled", COLOR_WHITE, 16);
//     print_text(50, 90, "IGDT Enabaled", COLOR_WHITE, 16);
//     print_text(50, 110, "Keyboard Enabaled", COLOR_WHITE, 16);
//     print_text(50, 130, "Simple User Shell Enabaled", COLOR_WHITE, 16);

// // Call `display_image` to draw `small_image` at position (100, 100)
//     display_image( 200, 200, small_image, 60, 40);

    // Hang the system
    hcf();
}





