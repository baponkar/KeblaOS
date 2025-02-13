/*
   Getting Framebuffer Information
*/

#include "../limine/limine.h"
#include "../driver/vga/vga_term.h"
#include "../util/util.h"

#include "framebuffer.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

size_t FRAMEBUFFER_WIDTH;
size_t FRAMEBUFFER_HEIGHT;
uint32_t *FRAMEBUFFER_PTR;


void get_framebuffer_info(){
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        halt_kernel();
    }

     // Ensure the framebuffer is initialized
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        halt_kernel();
    }

    // Fetch the first framebuffer
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    FRAMEBUFFER_PTR = (uint32_t*) framebuffer->address;

    FRAMEBUFFER_WIDTH = framebuffer->width;
    FRAMEBUFFER_HEIGHT = framebuffer->height;

}