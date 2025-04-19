
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern uint64_t LOWER_HALF_START_ADDR;
extern uint64_t LOWER_HALF_END_ADDR;

extern uint64_t HIGHER_HALF_START_ADDR;
extern uint64_t HIGHER_HALF_END_ADDR;
extern uint64_t HHDM_OFFSET;     // Transform Virtual to Physical Offset Present at Higher Half Memory


// Kernel Address
extern uint64_t KERNEL_VIR_BASE;
extern uint64_t KERNEL_PHYS_BASE;
extern uint64_t KERNEL_OFFSET;

extern uint64_t USABLE_START_PHYS_MEM;
extern uint64_t USABLE_END_PHYS_MEM;
extern uint64_t USABLE_LENGTH_PHYS_MEM;

extern uint64_t TOTAL_PHYS_MEMORY;

void get_kernel_address();
void get_hhdm_offset();

void get_phys_mem_map();
void set_usable_mem(size_t mem_entry_count, struct limine_memmap_entry **mem_entries;);
void get_total_phys_memory();
void get_set_memory();


