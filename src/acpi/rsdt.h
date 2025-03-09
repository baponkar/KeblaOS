#pragma once

#include "acpi.h"


// Root System Descriptor Table
struct rsdt{
    acpi_header_t header; // ACPI standard header
    uint32_t entries[];   // Array of pointers to ACPI tables
} __attribute__((packed));
typedef struct rsdt rsdt_t;


// Extended ACPI Header
struct xsdt{
    acpi_header_t header; // ACPI standard header
    uint64_t entries[];   // Array of 64-bit pointers to ACPI tables
} __attribute__((packed));
typedef struct xsdt xsdt_t;




