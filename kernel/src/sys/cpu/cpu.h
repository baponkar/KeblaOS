#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../../../ext_lib/limine-9.2.3/limine.h"
#include "../../arch/gdt/gdt.h"
#include "../../arch/gdt/tss.h"

#define MAX_CPUS   256            // Maximum number of CPUs supported
#define STACK_SIZE 4096 * 4       // 16 KB per core
#define TOTAL_GDT_ENTRIES 7       // 5 GDT(64 Bit) + 1 TSS (128 Bit)


typedef struct {
    
    uint64_t kernel_stack;      // Offset 0 — Kernel stack pointer
    uint64_t user_stack;        // Offset 8 — User stack pointer

    uint32_t lapic_id;          // Offset 16

    gdt_entry_t gdt_entries[TOTAL_GDT_ENTRIES];
    gdtr_t gdtr;
    tss_t tss;
    uint64_t tss_stack;

    uint8_t is_online;
    struct limine_smp_info *smp_info;
} cpu_data_t;


extern cpu_data_t cpu_datas[MAX_CPUS];  // Array indexed by CPU ID (APIC ID)

void switch_to_core(uint32_t target_lapic_id);

void init_bs_cpu_core();            // pic interrupt, gdt, tss, apic, paging, fpu
void start_bootstrap_cpu_core();    // apic, gdt, tss, paging, fpu

void start_ap_cpu_cores();
void init_all_cpu_cores();




