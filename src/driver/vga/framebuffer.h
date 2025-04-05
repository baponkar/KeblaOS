

#pragma once

#include <stdint.h>

extern uint64_t framebuffer_revision;
extern uint64_t framebuffer_count;
extern struct limine_framebuffer **framebuffers;

extern uint32_t *fb_address;

extern uint64_t fb_width;
extern uint64_t fb_height;
extern uint64_t fb_pitch;
extern uint16_t fb_bpp; // Bits per pixel

extern uint64_t fb_size;

void get_fb_info();

uint64_t get_fb_width();
uint64_t get_fb_height();
uint64_t get_fb_pitch();



