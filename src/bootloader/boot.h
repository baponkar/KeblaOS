#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../lib/string.h"

#include "../limine/limine.h"
#include "../driver/vga.h"

extern char *FIRMWARE_TYPE;



extern uint64_t CPU_COUNT;

extern uint64_t *RSDP_PTR;
extern uint64_t RSDP_REVISION;

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

extern uint64_t HHDM_REVISION;
extern uint64_t HHDM_OFFSET;

extern uint64_t KMEM_LOW_BASE;
extern uint64_t KMEM_UP_BASE;
extern uint64_t KMEM_LENGTH;

extern uint64_t V_KMEM_LOW_BASE;
extern uint64_t V_KMEM_UP_BASE;

extern uint64_t UMEM_LOW_BASE;
extern uint64_t UMEM_UP_BASE;
extern uint64_t UMEM_LENGTH;

extern uint64_t V_UMEM_LOW_BASE;
extern uint64_t V_UMEM_UP_BASE;

extern uint64_t TOTAL_MEMORY;



struct RSDP {
    char signature[8];    // "RSD PTR "
    uint8_t checksum;     // Checksum for the first 20 bytes
    char OEMID[6];        // OEM identifier
    uint8_t revision;     // ACPI revision (0 for ACPI 1.0, 2 for ACPI 2.0+)
    uint32_t rsdt_address; // Physical address of the RSDT (for ACPI 1.0)
    uint32_t length;      // Length of the RSDP (for ACPI 2.0+)
    uint64_t xsdt_address; // Physical address of the XSDT (for ACPI 2.0+)
    uint8_t checksum2;    // Checksum for the entire table (for ACPI 2.0+)
    uint8_t reserved[3];  // Reserved
};

// struct XSDT {
//     struct ACPI_SDT_Header header; // Standard ACPI table header
//     uint64_t entries[];            // Array of pointers to other ACPI tables
// };

void get_kernel_modules_info();
void print_kernel_modules_info();

void get_rsdp_info();
void print_rsdp_info();

void get_framebuffer_info();
void print_framebuffer_info();

void get_firmware_info();
void print_firmware_info();

void get_stack_info();
void print_stack_info();

void get_limine_info();
void print_limine_info();

void get_paging_mode_info();
void print_paging_mode_info();

void get_smp_info();
void print_smp_info();

void get_hhdm_info();
void print_hhdm_info();

void get_memory_map();
void print_memory_map();

void get_kernel_to_virtual_offset();
void print_kernel_to_virtual_offset();

void get_processor_name(char *name_buffer);
void print_processor_name();

void get_bootloader_info();
void print_bootloader_info();


void disk_read_sector(uint32_t sector, void* buffer);
void disk_write_sector(uint32_t sector, const void* buffer);
