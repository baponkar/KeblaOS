#include "framebuffer.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};


static struct limine_framebuffer *fb0;  // First Framebuffer


void init_framebuffer(){

    if (LIMINE_BASE_REVISION_SUPPORTED == false || framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        for (;;) {
            asm ("hlt");
        }
    }

    fb0 = framebuffer_request.response->framebuffers[0];

}

uint64_t get_fb_revision(){
    if (LIMINE_BASE_REVISION_SUPPORTED == false || framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        return 0;
    }
    return framebuffer_request.response->revision;
}

uint64_t get_fb_count(){
    if (LIMINE_BASE_REVISION_SUPPORTED == false || framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        return 0;
    }
    return framebuffer_request.response->framebuffer_count;
}

struct limine_framebuffer **get_all_fb_ptr(){
    if (LIMINE_BASE_REVISION_SUPPORTED == false || framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        return NULL;
    }
    return framebuffer_request.response->framebuffers;
}

struct limine_framebuffer *get_first_fb_ptr(){
    if (LIMINE_BASE_REVISION_SUPPORTED == false || framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        return NULL;
    }
    return framebuffer_request.response->framebuffers[0];
}

void *get_fb0_address(){
    struct limine_framebuffer *fb0 = get_first_fb_ptr();
    if(!fb0) return 0;
    return (struct limine_framebuffer *) fb0->address;
}

uint64_t get_fb0_width(){
    struct limine_framebuffer *fb0 = get_first_fb_ptr();
    if(!fb0) return 0;
    return fb0->width;
}

uint64_t get_fb0_height(){
    struct limine_framebuffer *fb0 = get_first_fb_ptr();
    if(!fb0) return 0;
    return fb0->height;
}

uint64_t get_fb0_pitch(){
    struct limine_framebuffer *fb0 = get_first_fb_ptr();
    if(!fb0) return 0;
    return fb0->pitch;
}

uint16_t get_fb0_bpp(){
    struct limine_framebuffer *fb0 = get_first_fb_ptr();
    if(!fb0) return 0;
    return fb0->bpp;
}

void *get_fb0_unused_ptr(){
    struct limine_framebuffer *fb0 = get_first_fb_ptr();
    if(!fb0) return NULL;
    return fb0->unused;
}

uint64_t get_fb0_edid_size(){
    struct limine_framebuffer *fb0 = get_first_fb_ptr();
    if(!fb0) return 0;
    return fb0->edid_size;
}

void *get_fb0_edid(){
    struct limine_framebuffer *fb0 = get_first_fb_ptr();
    if(!fb0) return NULL;
    return fb0->edid;
}


