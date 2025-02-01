#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#include "boot.h"

struct rsdp_1 {
    uint8_t revision;
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint32_t rsdt_address;
} __attribute__ ((packed));
typedef struct rsdp_1 rsdp_1_t;


struct rsdp_2 {
    rsdp_1_t rsdp_1;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__ ((packed));
typedef struct rsdp_2 rsdp_2_t;


struct acpi_sdt_header {
    uint8_t revision;
    char signature[4];
    uint8_t checksum;
    char OEMID[6];
    uint32_t length;
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
};
typedef struct acpi_sdt_header acpi_sdt_header_t;


struct rsdp {
    acpi_sdt_header_t acpi_sdt_header; //signature "RSDP"
    uint32_t sdt_addresses[];
};
typedef struct rsdp rsdp_t;


struct xsdt {
    acpi_sdt_header_t acpi_sdt_header; // Standard ACPI table header
    uint64_t sdt_addresses[];            // Array of pointers to other ACPI tables
};
typedef struct xsdt xsdt_t;

void init_acpi();


void qemu_poweroff();
void qemu_reboot();
