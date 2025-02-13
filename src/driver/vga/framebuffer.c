
#include <stdbool.h>
#include <stddef.h>
#include "../../util/util.h"
#include "../../limine/limine.h"

#include "framebuffer.h"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

uint64_t framebuffer_revision;
uint64_t framebuffer_count;
struct limine_framebuffer **framebuffers;

uint32_t *fb_address;

uint64_t fb_width;
uint64_t fb_height;
uint64_t fb_pitch;
uint16_t fb_bpp; // Bits per pixel

uint8_t fb_memory_model;

uint8_t fb_red_mask_size;
uint8_t fb_red_mask_shift;
uint8_t fb_green_mask_size;
uint8_t fb_green_mask_shift;
uint8_t fb_blue_mask_size;
uint8_t fb_blue_mask_shift;

uint8_t fb_unused[7];
uint64_t fb_edid_size;
void *fb_edid;

/* Response revision 1 */
uint64_t fb_mode_count;
struct limine_video_mode **fb_modes;


void get_fb_info(){

    if (LIMINE_BASE_REVISION_SUPPORTED == false || framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        halt_kernel();
    }

    framebuffer_revision = framebuffer_request.response->revision;          // Framebuffer Revision
    framebuffer_count = framebuffer_request.response->framebuffer_count;    // Total framebuffers
    framebuffers = framebuffer_request.response->framebuffers;              // Start of framebuffer pointer

    // Getting first framebbuffer
    struct limine_framebuffer *framebuffer0 = framebuffer_request.response->framebuffers[0];
    
    fb_address = (uint32_t *) framebuffer0->address;
    fb_width = framebuffer0->width;
    fb_height = framebuffer0->height;
    fb_pitch = framebuffer0->pitch;
    fb_bpp = framebuffer0->bpp;

    fb_memory_model = framebuffer0->memory_model;

    fb_red_mask_size = framebuffer0->red_mask_size;
    fb_red_mask_shift = framebuffer0->red_mask_shift;
    fb_green_mask_size = framebuffer0->green_mask_size;
    fb_green_mask_shift = framebuffer0->green_mask_shift;
    fb_blue_mask_size = framebuffer0->blue_mask_size;
    fb_blue_mask_shift = framebuffer0->blue_mask_shift;

    // fb_unused = framebuffer0->unused;
    fb_edid_size = framebuffer0->edid_size;
    fb_edid = framebuffer0->edid;
    
    fb_mode_count = framebuffer0->mode_count;
    fb_modes = framebuffer0->modes;
}

