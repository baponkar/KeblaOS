#pragma once

#include <stdint.h>

#include "acpi.h"   // For acpi_header_t


// PCI Devices
struct mcfg{
    acpi_header_t header; // ACPI table header
    uint64_t reserved;
} __attribute__((packed));
typedef struct mcfg mcfg_t;



void parse_mcfg(acpi_header_t *table);

