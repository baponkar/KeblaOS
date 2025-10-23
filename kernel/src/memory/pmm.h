
#pragma once

#include <stdint.h>
#include "detect_memory.h"

#define FRAME_SIZE 4096   // 4 KB
#define BITMAP_SIZE 64 // 64 bits = 8 bytes

// Finding index and offset from the bit number
#define INDEX_FROM_BIT_NO(x)(x / BITMAP_SIZE)
#define OFFSET_FROM_BIT_NO(x)(x % BITMAP_SIZE)

// Making bit no from index and offset
#define CONVERT_BIT_NO(idx, off) (idx * BITMAP_SIZE + off)

// Converting bit number to address
#define BIT_NO_TO_ADDR(bit_no) (USABLE_START_PHYS_MEM + (bit_no * FRAME_SIZE))

// Converting physical address to bit number
// #define PHYS_ADDR_TO_BIT_NO(phy_addr)(((phy_addr) < (USABLE_START_PHYS_MEM)) ? 0 : ((phy_addr - USABLE_START_PHYS_MEM) / FRAME_SIZE))
// In pmm.h - Revised version
#define PHYS_ADDR_TO_BIT_NO(phy_addr) ( \
    ((phy_addr) < USABLE_START_PHYS_MEM || (phy_addr) >= (USABLE_START_PHYS_MEM + USABLE_LENGTH_PHYS_MEM)) ? \
    (uint64_t)-1 : /* Return invalid value for out-of-range addresses */ \
    (((phy_addr) - USABLE_START_PHYS_MEM) / FRAME_SIZE) \
)

// Finding the maximum frame index from the memory size.
#define MAX_FRAME_INDEX(memory_size) (memory_size / (BITMAP_SIZE * FRAME_SIZE))

extern uint64_t *frames; // start of bitset frames
extern uint64_t nframes; // Total frames

void set_frame(uint64_t frame_addr);
void clear_frame(uint64_t frame_addr);
uint64_t test_frame(uint64_t frame_addr);
int64_t free_frame_bit_no();

void init_pmm();

void test_pmm();

