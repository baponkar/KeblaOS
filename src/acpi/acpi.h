#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../bootloader/boot.h"
#include "../driver/ports.h"

void poweroff();
void qemu_poweroff();
void reboot();

// ACPI 1.0 RSDP descriptor
struct RSDPDescriptor {
    char Signature[8];  // "RSD PTR "
    uint8_t Checksum;   // Checksum for the first 20 bytes
    char OEMID[6];      // OEM identifier
    uint8_t Revision;   // ACPI revision (0 for ACPI 1.0, 2 for ACPI 2.0+)
    uint32_t RsdtAddress;   // Physical address of the RSDT (for ACPI 1.0)
} __attribute__ ((packed)); // This tells the compiler to not add any padding to the struct


// ACPI 2.0+ RSDP descriptor
struct RSDP2Descriptor {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;

    uint32_t Length;        // Length of the RSDP (for ACPI 2.0+)
    uint64_t XSDTAddress;   // Physical address of the XSDT (for ACPI 2.0+)
    uint8_t ExtendedChecksum;   // Checksum for the entire table (for ACPI 2.0+)
    uint8_t Reserved[3];    // Reserved
};

struct ACPISDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
};


bool validate_RSDP(char *byte_array, size_t size);