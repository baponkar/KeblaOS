
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


// Start Stack Memory
extern uint64_t STACK_MEM_SIZE;

// LIMINE_PAGING_MODE_X86_64_4LVL = 0
// LIMINE_PAGING_MODE_X86_64_5LVL = 1
extern uint64_t paging_mode;

// Lower Half Virtual address
extern uint64_t LOWER_HALF_START_ADDR;
extern uint64_t LOWER_HALF_END_ADDR;

// Higher Half Virtual address
extern uint64_t HIGHER_HALF_START_ADDR;
extern uint64_t HIGHER_HALF_END_ADDR;
extern uint64_t HHDM_OFFSET;     // Transform Virtual to Physical Offset Present at Higher Half Memory


// Kernel Address
extern uint64_t KERNEL_VIR_BASE;
extern uint64_t KERNEL_PHYS_BASE;
extern uint64_t KERNEL_OFFSET;

// Changable Physical Memory Head address
extern volatile uint64_t phys_mem_head;

// Usable Physical Memory Address
extern uint64_t USABLE_START_PHYS_MEM;
extern uint64_t USABLE_END_PHYS_MEM;
extern uint64_t USABLE_LENGTH_PHYS_MEM;

// Total Physical Memory address present in the device
extern uint64_t TOTAL_PHYS_MEMORY;

void get_stack_mem_info();
void get_paging_mode();
void get_kernel_address();
void get_hhdm_offset();

void get_phys_mem_map();
void set_usable_mem();
void get_total_phys_memory();
void get_set_memory();


