
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../util/util.h"
#include "../limine/limine.h"
#include "../bootloader/boot.h"
#include "kmalloc.h"
#include "paging.h"
#include "../bootloader/boot.h"
#include  "../driver/vga.h"
#include "../kernel/kernel.h"
#include "../lib/assert.h"

#define FRAME_SIZE 4096   // 4 KB
#define BITMAP_SIZE 64 // 64 bits = 8 bytes

// Getting the index and offset from the bit number
// #define INDEX_FROM_BIT_NO(x)((x+BITMAP_SIZE+1)/BITMAP_SIZE) // ceiling division, ensuring that if x is less than or equal to BITMAP_SIZE, the result is 1.
#define INDEX_FROM_BIT_NO(x)(x / BITMAP_SIZE)
#define OFFSET_FROM_BIT_NO(x)(x % BITMAP_SIZE)

// Converting bit no from index and offset
#define CONVERT_BIT_NO(idx, off) (idx * BITMAP_SIZE + off)

// Converting bit number to address
#define BIT_NO_TO_ADDR(bit_no) (bit_no * FRAME_SIZE)

// Converting address to bit number
#define ADDR_TO_BIT_NO(addr) ((addr - PHYSICAL_TO_VIRTUAL_OFFSET) / FRAME_SIZE)

// Finding the maximum frame index from the memory size.
#define MAX_FRAME_INDEX(memory_size) (memory_size / (BITMAP_SIZE * FRAME_SIZE))

extern uint64_t *frames; // start of bitset frames
extern uint64_t nframes; // Total frames

void set_frame(uint64_t frame_addr);
void clear_frame(uint64_t frame_addr);
uint64_t test_frame(uint64_t frame_addr);
uint64_t free_frame_bit_no();

void init_pmm();
void test_pmm();

