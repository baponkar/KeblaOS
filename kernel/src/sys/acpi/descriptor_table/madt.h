#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../acpi.h"   // For acpi_header_t

// Multiple APIC Description Table (MADT)
struct madt{
    acpi_header_t header;           // ACPI standard header
    uint32_t local_apic_address;    // Physical address of LAPIC
    uint32_t flags;
} __attribute__((packed));
typedef struct madt madt_t;

// MADT entry header
struct madt_entry {
    uint8_t type;   // Type of entry
    uint8_t length; // Length of this entry
    uint8_t data[]; // Variable-length data
};
typedef struct madt_entry madt_entry_t;

// I/O APIC entry
struct madt_ioapic {
    uint8_t type;          // 1 (I/O APIC entry)
    uint8_t length;        // Size of this entry (usually 12 bytes)
    uint8_t ioapic_id;     // I/O APIC ID
    uint8_t reserved;      // Reserved field
    uint32_t ioapic_addr;  // Memory-mapped I/O APIC Base Address
    uint32_t gsi_base;     // Global System Interrupt Base
} __attribute__((packed));
typedef struct madt_ioapic madt_ioapic_t;


// Interrupt Source Override entry
typedef struct madt_iso {
    uint8_t type;
    uint8_t length;
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t gsi;
    uint16_t flags;
} __attribute__((packed)) madt_iso_t;


// Local APIC Address Override entry
typedef struct madt_lapic_override {
    uint8_t type;
    uint8_t length;
    uint16_t reserved;
    uint64_t lapic_addr;
} __attribute__((packed)) madt_lapic_override_t;

extern madt_t *madt_addr;

void parse_madt();
