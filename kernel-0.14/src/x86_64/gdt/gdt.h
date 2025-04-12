#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "tss.h"


union gdt_entry
{   
    struct { // 64-Bit
        uint16_t limit_low;       // Lower 16 bits of the segment limit
        uint16_t base_low;        // Lower 16 bits of the base address
        uint8_t base_middle;      // Next 8 bits of the base address
        uint8_t access;           // Access byte
        uint8_t granularity;      // Flags and upper limit
        uint8_t base_high;        // Next 8 bits of the base address
    };
    struct { // 64-Bit
        uint32_t base_upper;     // Upper 32 bits of the base address (for 64-bit TSS)
        uint32_t reserved;       // Reserved, must be zero
    };
};
typedef union gdt_entry gdt_entry_t;


// Structure for GDTR
struct gdtr {
    uint16_t limit;
    uint64_t base; // Use uint64_t for 64-bit systems
} __attribute__((packed));
typedef struct gdtr gdtr_t;




// #define GDT_ENTRIES_COUNT 7 // 5 GDT(64 Bit) + 1 TSS (128 Bit)
// typedef struct {
//     gdt_entry_t gdt[GDT_ENTRIES_COUNT];  // Each core's GDT
//     gdtr_t gdtr;
//     tss_t tss;                            // Core's Task State Segment
//     uint64_t kernel_stack;                // Kernel stack for Ring 0
// } cpu_data_t;


// void init_core_gdt_tss(int core_id);
// void init_bootstrap_gdt_tss(int bootstrap_core_id);
// void init_application_core_gdt_tss(int start_core_id, int end_core_id);


void gdt_tss_init();

void print_gdt_entry(uint16_t selector);





