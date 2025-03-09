#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "acpi.h"   // For acpi_header_t

// Multiple APIC Description Table (MADT)
struct madt{
    acpi_header_t header; // ACPI standard header
    uint32_t local_apic_address; // Physical address of LAPIC
    uint32_t flags;
} __attribute__((packed));
typedef struct madt madt_t;


void parse_madt();
