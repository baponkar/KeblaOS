
#pragma once

#include <stdint.h>

#include "../acpi.h"   // For acpi_header_t


struct mcfg{
    acpi_header_t header; // ACPI table header
    uint64_t reserved;
} __attribute__((packed));
typedef struct mcfg mcfg_t;

// MCFG allocation entry structure (follows immediately after mcfg_t)
typedef struct mcfg_allocation {
    uint64_t base_address;   // Base physical address of PCI configuration space
    uint16_t segment_group;  // PCI segment group number
    uint8_t  start_bus;      // Starting PCI bus number in this segment
    uint8_t  end_bus;        // Ending PCI bus number in this segment
    uint32_t reserved;
} __attribute__((packed)) mcfg_alloc_t;

void parse_mcfg(mcfg_t *mcfg);






