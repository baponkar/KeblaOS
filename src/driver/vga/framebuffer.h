

#pragma once

#include <stdint.h>

extern uint32_t *fb_address;

extern uint64_t fb_width;
extern uint64_t fb_height;
extern uint64_t fb_pitch;
extern uint16_t fb_bpp; // Bits per pixel

void get_fb_info();





