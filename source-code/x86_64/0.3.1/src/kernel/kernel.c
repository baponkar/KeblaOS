#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../util/util.h"
#include "../../limine/limine.h"

#include "../driver/vga/vga.h"
#include "../driver/keyboard.h"

#include "../usr/shell.h"

#include "../gdt/gdt.h"
#include "../idt/idt.h"
#include "../idt/timer.h"

#include "info.h"

extern uint64_t ticks;
extern uint64_t seconds;

uint32_t *fb_ptr = NULL;

size_t SCREEN_WIDTH;
size_t SCREEN_HEIGHT;

size_t MIN_LINE_NO;
size_t MAX_LINE_NO;

size_t MIN_COLUMN_NO;
size_t MAX_COLUMN_NO;

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


void kmain(void) {

    get_framebuffer_info();
    cls();

    extern uint32_t KeblaOS_icon_320x200x32[];
    display_image( (SCREEN_WIDTH/2 - KEBLAOS_ICON_320X200X32_WIDTH/2), 5, KeblaOS_icon_320x200x32, KEBLAOS_ICON_320X200X32_WIDTH, KEBLAOS_ICON_320X200X32_HEIGHT);


    init_gdt();
    init_idt();
    init_timer();
    initKeyboard();



   

    hcf();
}


void get_framebuffer_info(){
    // Ensure the framebuffer is initialized
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    fb_ptr = (uint32_t* ) framebuffer->address;

    SCREEN_WIDTH = framebuffer->width;   // 1024
    SCREEN_HEIGHT = framebuffer->height; // 768

    MIN_LINE_NO = 0;
    MAX_LINE_NO = (SCREEN_HEIGHT / (DEFAULT_FONT_HEIGHT + DEFAULT_TEXT_LINE_GAP)) - 1;

    MIN_COLUMN_NO = 0;
    MAX_COLUMN_NO = (SCREEN_WIDTH / DEFAULT_FONT_WIDTH) - 1;
}


