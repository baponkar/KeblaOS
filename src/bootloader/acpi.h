#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "boot.h"

void qemu_poweroff();
void qemu_reboot();

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


typedef struct 
{
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
}__attribute__((packed)) GenericAddressStructure_t;


typedef struct
{
    acpi_header_t header; // ACPI table header
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;

    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;

    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;

    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;

    uint8_t  Reserved2;
    uint32_t Flags;

    // 12 byte structure; see below for details
    GenericAddressStructure_t ResetReg;

    uint8_t  ResetValue;
    uint8_t  Reserved3[3];
  
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;

    GenericAddressStructure_t X_PM1aEventBlock;
    GenericAddressStructure_t X_PM1bEventBlock;
    GenericAddressStructure_t X_PM1aControlBlock;
    GenericAddressStructure_t X_PM1bControlBlock;
    GenericAddressStructure_t X_PM2ControlBlock;
    GenericAddressStructure_t X_PMTimerBlock;
    GenericAddressStructure_t X_GPE0Block;
    GenericAddressStructure_t X_GPE1Block;
} __attribute__((packed)) fadt_t;


void *find_acpi_table();
void parse_acpi_table(void *table_addr);
void parse_madt(acpi_header_t *table);
void parse_mcfg(acpi_header_t *table);
void parse_fadt(acpi_header_t *table);
void acpi_poweroff();
void acpi_reboot();
void init_acpi();
