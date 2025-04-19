#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../bootloader/boot.h"

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


// RSDP (Root System Description Pointer
struct rsdp{
    char signature[8];     // "RSD PTR "
    uint8_t checksum;      // Checksum of first 20 bytes
    char oem_id[6];        // OEM Identifier
    uint8_t revision;      // 0 for ACPI 1.0, 2 for ACPI 2.0+
    uint32_t rsdt_address; // Address of RSDT (ACPI 1.0)
} __attribute__((packed));
typedef struct rsdp rsdp_t;


// Extended RSDP
struct rsdp_ext{
    rsdp_t first_part;     // First 20 bytes of ACPI 1.0 structure
    uint32_t length;       // Total size of this structure
    uint64_t xsdt_address; // Address of XSDT (64-bit)
    uint8_t checksum;      // Checksum of entire structure
} __attribute__((packed)) ;
typedef struct rsdp_ext rsdp_ext_t;


// ACPI (Advanced Configuration and Power Interface) Header
struct acpi_header {
    char signature[4];    // Table signature (e.g., "APIC", "FACP")
    uint32_t length;      // Table length
    uint8_t revision;     // ACPI version
    uint8_t checksum;     // Checksum of table
    char oem_id[6];       // OEM Identifier
    char oem_table_id[8]; // OEM Table Identifier
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));
typedef struct acpi_header acpi_header_t;

struct GenericAddressStructure
{
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
}__attribute__((packed));
typedef struct GenericAddressStructure GenericAddressStructure_t;


void find_acpi_table_pointer();
void validate_rsdp_table(rsdp_t *rsdp);

int is_acpi_enabled();
void acpi_enable();
void init_acpi();


