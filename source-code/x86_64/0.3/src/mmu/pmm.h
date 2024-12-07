#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../util/util.h"

#include "../../limine/limine.h"

#include "../stdlib/stdio.h"

#include  "../driver/vga/vga.h"
#include "../kernel/kernel.h"

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*8))
#define OFFSET_FROM_BIT(a) (a%(8*8))

extern uint64_t *frames;
extern uint64_t nframes;


void set_frame(uint64_t frame_addr);
void clear_frame(uint64_t frame_addr);
uint64_t test_frame(uint64_t frame_addr);
uint64_t first_frame();

