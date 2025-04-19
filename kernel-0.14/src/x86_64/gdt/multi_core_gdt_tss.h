#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "gdt.h"
#include "tss.h"

#define TOTAL_GDT_ENTRIE 7                // 5 GDT(64 Bit) + 1 TSS (128 Bit)

typedef struct {
    gdt_entry_t gdt_entries[TOTAL_GDT_ENTRIE];   // Each core's GDT
    gdtr_t gdtr;
    tss_t tss;                            // Core's Task State Segment
    uint64_t kernel_stack;                // Kernel stack for Ring 0
} cpu_data_t;


void init_gdt_tss_in_cpu(size_t cpu_id);






