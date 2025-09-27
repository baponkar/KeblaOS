#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../../../ext_lib/limine-9.2.3/limine.h"

void init_framebuffer();

uint64_t get_fb_revision();
uint64_t get_fb_count();

struct limine_framebuffer **get_all_fb_ptr();
struct limine_framebuffer *get_first_fb_ptr();

void *get_fb0_address();
uint64_t get_fb0_width();
uint64_t get_fb0_height();
uint64_t get_fb0_pitch();
uint16_t get_fb0_bpp();
void *get_fb0_unused_ptr();
uint64_t get_fb0_edid_size();
void *get_fb0_edid();






