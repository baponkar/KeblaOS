#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


union gdt_entry
{   // 64-Bit
    struct {
        uint16_t limit_low;       // Lower 16 bits of the segment limit
        uint16_t base_low;        // Lower 16 bits of the base address
        uint8_t base_middle;      // Next 8 bits of the base address
        uint8_t access;           // Access byte
        uint8_t granularity;      // Flags and upper limit
        uint8_t base_high;        // Next 8 bits of the base address
    };
    struct {
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


// Add TSS entry structure
struct tss{ // 104 bytes is the minimum size of a TSS
    uint32_t reserved0;
    uint64_t rsp0;  // Ring 0 Stack Pointer
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;  // Interrupt Stack Entries
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed));
typedef struct tss tss_t;

#define GDT_ENTRIES_COUNT 7 // 5 GDT(64 Bit) + 1 TSS (128 Bit)
typedef struct {
    gdt_entry_t gdt[GDT_ENTRIES_COUNT];  // Each core's GDT
    gdtr_t gdtr;
    tss_t tss;                            // Core's Task State Segment
    uint64_t kernel_stack;                // Kernel stack for Ring 0
} cpu_data_t;

void gdt_setup(gdt_entry_t gdt[], uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void gdt_setup_sysseg(gdt_entry_t gdt[], uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void load_gdt_tss(cpu_data_t *core);
void start_bootstrap_gdt_tss();
void init_gdt_tss(cpu_data_t *core);
void init_all_gdt_tss();
void core_init(int core);
void start_secondary_cores();
