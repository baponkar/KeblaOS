#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


struct gdt_entry
{// 128-Bit
    uint16_t limit_low;       // Lower 16 bits of the segment limit
    uint16_t base_low;        // Lower 16 bits of the base address
    uint8_t base_middle;      // Next 8 bits of the base address
    uint8_t access;           // Access byte
    uint8_t granularity;      // Flags and upper limit
    uint8_t base_high;        // Next 8 bits of the base address

    uint32_t base_upper;     // Upper 32 bits of the base address (for 64-bit TSS)
    uint32_t reserved;       // Reserved, must be zero
}__attribute__((packed, aligned(16)));
typedef struct gdt_entry gdt_entry_t;


// Structure for GDTR
struct gdtr {
    uint16_t limit;
    uint64_t base; // Use uint64_t for 64-bit systems
} __attribute__((packed));
typedef struct gdtr gdtr_t;


// Add TSS entry structure
struct tss_entry{ // 9*64 = 576 bit = 72 byte = 0x48
    uint32_t reserved0;
    uint64_t rsp0;  // Ring 0 Stack Pointer
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;  // Interrupt Stack Table (optional)
    uint64_t ist2;
    uint64_t ist3;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed));
typedef struct tss_entry tss_entry_t;




void init_gdt_bootstrap_cpu();
void init_tss_bootstrap_cpu();
void start_bootstrap_gdt_tss();

void init_gdt_ap_cpu(uint64_t core_id);

gdtr_t* get_gdt_ptr_for_ap(uint64_t core_id);