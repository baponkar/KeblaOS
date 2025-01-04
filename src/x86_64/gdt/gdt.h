#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../driver/vga.h"


struct gdt_entry
{// 64-Bit
    uint16_t limit_low;       // Lower 16 bits of the segment limit
    uint16_t base_low;        // Lower 16 bits of the base address
    uint8_t base_middle;      // Next 8 bits of the base address
    uint8_t access;           // Access byte
    uint8_t granularity;      // Flags and upper limit
    uint8_t base_high;        // Next 8 bits of the base address
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

// Structure for GDTR
struct gdtr {
    uint16_t limit;
    uint64_t base; // Use uint64_t for 64-bit systems
} __attribute__((packed));
typedef struct gdtr gdtr_t;


void gdt_setup( uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void gdt_setup_sysseg( uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void init_gdt();
void check_gdt();

