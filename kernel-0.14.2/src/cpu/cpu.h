#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../../limine-8.6.0/limine.h"
#include "../x86_64/gdt/gdt.h"
#include "../x86_64/gdt/tss.h"

#define MAX_CPUS   256            // Maximum number of CPUs supported
#define STACK_SIZE 4096 * 4       // 16 KB per core
#define TOTAL_GDT_ENTRIES 7       // 5 GDT(64 Bit) + 1 TSS (128 Bit)


typedef struct {
    uint32_t lapic_id;                      // LAPIC ID of the core
    gdt_entry_t gdt_entries[TOTAL_GDT_ENTRIES];   // Each core's GDT
    gdtr_t gdtr;                            // Core's GDT Register
    tss_t tss;                              // Core's Task State Segment
    uint64_t tss_stack;                     // The stack pointer for the TSS
    uint8_t is_online;                      // Flag to indicate if the core is online
    struct limine_smp_info *smp_info;       // Pointer to the SMP info structure
    uint64_t cpu_stack;                     // Pointer to the CPU stack
} cpu_data_t;


void switch_to_core(uint32_t target_lapic_id);

void start_bootstrap_cpu_core();
void start_ap_cpu_cores();
void init_all_cpu_cores();




