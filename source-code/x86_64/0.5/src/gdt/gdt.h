
#pragma once

#include <stdint.h>
#include "../driver/vga/vga.h"

struct gdt_entry
{// 128 Bit
    uint16_t limit_low;       // Lower 16 bits of the segment limit
    uint16_t base_low;        // Lower 16 bits of the base address
    uint8_t base_middle;      // Next 8 bits of the base address
    uint8_t access;           // Access byte
    uint8_t granularity;      // Flags and upper limit
    uint8_t base_high;        // Next 8 bits of the base address
    uint32_t base_upper;      // Upper 32 bits of the base address
    uint32_t reserved;        // Reserved (must be zero)
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;


void gdt_setup( int idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void init_gdt();
void check_gdt();


