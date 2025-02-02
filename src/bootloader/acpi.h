#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "boot.h"

typedef enum {
    UNSPECIFIED,
    DESKTOP,
    MOBILE,
    WORKSTATION,
    ENTERPRISE_SERVER,
    SOHO_SERVER,
    APLLIANCE_PC,
    PERFORMANCE_SERVER,
    RESERVED
}device_type_t;

typedef struct {
    char signature[8];     // "RSD PTR "
    uint8_t checksum;      // Checksum of first 20 bytes
    char oem_id[6];        // OEM Identifier
    uint8_t revision;      // 0 for ACPI 1.0, 2 for ACPI 2.0+
    uint32_t rsdt_address; // Address of RSDT (ACPI 1.0)
} __attribute__((packed)) rsdp_t;

typedef struct {
    rsdp_t first_part;     // First 20 bytes of ACPI 1.0 structure
    uint32_t length;       // Total size of this structure
    uint64_t xsdt_address; // Address of XSDT (64-bit)
    uint8_t checksum;      // Checksum of entire structure
} __attribute__((packed)) rsdp_ext_t;

typedef struct {
    char signature[4];    // Table signature (e.g., "APIC", "FACP")
    uint32_t length;      // Table length
    uint8_t revision;     // ACPI version
    uint8_t checksum;     // Checksum of table
    char oem_id[6];       // OEM Identifier
    char oem_table_id[8]; // OEM Table Identifier
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_header_t;

typedef struct {
    acpi_header_t header; // ACPI standard header
    uint32_t entries[];   // Array of pointers to ACPI tables
} __attribute__((packed)) rsdt_t;

typedef struct {
    acpi_header_t header; // ACPI standard header
    uint64_t entries[];   // Array of 64-bit pointers to ACPI tables
} __attribute__((packed)) xsdt_t;

typedef struct {
    acpi_header_t header; // ACPI standard header
    uint32_t local_apic_address; // Physical address of LAPIC
    uint32_t flags;
} __attribute__((packed)) madt_t;

typedef struct {
    acpi_header_t header; // ACPI table header
    uint64_t reserved;
} __attribute__((packed)) mcfg_t;

extern void *fadt_addr;
extern void *madt_addr;

void *find_acpi_table();
void validate_acpi_table(void *table_addr);
void parse_acpi_table(void *table_addr);

void parse_mcfg(acpi_header_t *table);
int is_acpi_enabled();
void acpi_enable();
void init_acpi();

