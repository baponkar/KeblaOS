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

extern uint64_t KERNEL_MEM_START_ADDRESS;
extern uint64_t KERNEL_MEM_END_ADDRESS;
extern uint64_t KERNEL_MEM_LENGTH;

extern uint64_t USER_START_ADDRESS;
extern uint64_t USER_END_ADDRESS;
extern uint64_t USER_MEM_LENGTH;

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

void get_kernel_modules_info(void);
void get_rsdp_info(void);
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
