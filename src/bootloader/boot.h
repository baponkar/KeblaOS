#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../limine/limine.h"
#include "../driver/vga.h"

extern char *FIRMWARE_TYPE;

extern uint64_t CPU_COUNT;

extern char *BOOTLOADER_NAME;
extern char *BOOTLOADER_VERSION;

extern uint64_t STACK_SIZE;

extern uint64_t MULTIPROCESSOR_REVISION;
extern uint64_t MULTIPROCESSOR_OFFSET;

extern char *LIMINE_PAGING_MODE;

extern uint64_t KERNEL_ADDRESS_REVISION;
extern uint64_t PHYSICAL_BASE;
extern uint64_t VIRTUAL_BASE;
extern uint64_t PHYSICAL_TO_VIRTUAL_OFFSET;

extern uint64_t HIGHER_HALF_DIRECT_MAP_REVISION;
extern uint64_t HIGHER_HALF_DIRECT_MAP_OFFSET;

extern struct limine_memmap_request memmap_request;
extern uint64_t entry_count;
extern struct limine_memmap_entry **entries;
extern uint64_t TOTAL_MEMORY;

void get_firmware_info(void);
void get_stack_info(void);
void get_limine_info(void);
void get_paging_mode_info(void);
void get_smp_info(void);
void get_hhdm_info(void);
void get_memory_map(void);
void print_memory_map(void);
void get_kernel_to_virtual_offset(void);

void get_bootloader_info(void);
void print_bootloader_info(void);
