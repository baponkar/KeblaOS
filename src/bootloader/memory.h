#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


extern uint64_t KMEM_UP_BASE;
extern uint64_t KMEM_LOW_BASE;
extern uint64_t KMEM_LENGTH;

extern uint64_t UMEM_LOW_BASE;
extern uint64_t UMEM_UP_BASE;
extern uint64_t UMEM_LENGTH;

extern uint64_t TOTAL_MEMORY;
extern uint64_t USABLE_MEMORY;
extern uint64_t RESERVED_MEMORY;
extern uint64_t BAD_MEMORY;
extern uint64_t BOOTLOADER_RECLAIMABLE_MEMORY;
extern uint64_t ACPI_RECLAIMABLE_MEMORY;
extern uint64_t ACPI_NVS_MEMORY;
extern uint64_t FRAMEBUFFER_MEMORY;
extern uint64_t KERNEL_MODULES_MEMORY;
extern uint64_t UNKNOWN_MEMORY;


extern uint64_t KERNEL_ADDRESS_REVISION;
extern uint64_t PHYSICAL_BASE;
extern uint64_t VIRTUAL_BASE;
extern uint64_t PHYSICAL_TO_VIRTUAL_OFFSET;

extern uint64_t V_KMEM_LOW_BASE;
extern uint64_t V_KMEM_UP_BASE;

extern uint64_t V_UMEM_LOW_BASE;
extern uint64_t V_UMEM_UP_BASE;

extern uint64_t HHDM_REVISION;
extern uint64_t HHDM_OFFSET;


void get_hhdm_info();
void print_hhdm_info();

void get_memory_map();
void print_memory_map();

void get_kernel_to_virtual_offset();
void print_kernel_to_virtual_offset();


void get_memory_info();

