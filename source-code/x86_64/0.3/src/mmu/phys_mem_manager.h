#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Physical Memory Manager by bitmap
#define PAGE_SIZE 4097

#define BIT_PER_ROW 8 // one byte

struct bitmap_loc{
    uint64_t row;
    uint64_t col;
}
typedef struct bitmap_loc bitmap_loc_t;