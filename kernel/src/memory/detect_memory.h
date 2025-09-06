
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Usermode Virtual address
#define LOWER_HALF_START_ADDR 0x1000                   // 0 MB
#define LOWER_HALF_END_ADDR   0x00007FFFFFFFFFFF    // 128 TB

// Kernelmode Virtuai address
#define HIGHER_HALF_START_ADDR 0xFFFF800000001000   // 17 PiB
#define HIGHER_HALF_END_ADDR   0xFFFFFFFFFFFFFFFF   // 16 EiB

// Start Stack Memory
extern uint64_t STACK_MEM_SIZE;

// LIMINE_PAGING_MODE_X86_64_4LVL = 0
// LIMINE_PAGING_MODE_X86_64_5LVL = 1
extern uint64_t paging_mode;

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


