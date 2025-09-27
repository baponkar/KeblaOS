
/*
MCFG (Memory-Mapped Configuration Table)

https://wiki.osdev.org/PCI_Express
*/

#include "../../../lib/stdio.h"

#include "../../../memory/vmm.h"
#include "../../../memory/paging.h"

#include "mcfg.h"

extern bool debug_on;

mcfg_t *mcfg; // Defined in acpi.c

uint32_t pci_read_mmconfig(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    
    if(!mcfg) {
        if (debug_on) printf("[ACPI] MCFG table is NULL, cannot read PCI config space!\n");
        return 0xFFFFFFFF;
    }
    
    // Calculate the address in memory mapped configuration space.
    uint64_t config_addr = mcfg->reserved +
        (((uint64_t)bus << 20) | ((uint64_t)device << 15) | ((uint64_t)function << 12) | (offset & 0xFFF));

    // The vendor/device ID register is at offset 0 in the configuration space.
    volatile uint32_t *ptr = (volatile uint32_t *)config_addr;
    return *ptr;
}

void pci_write_mmconfig(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    
    if(!mcfg) {
        if(debug_on) printf("[ACPI] MCFG table is NULL, cannot write to MMCONFIG space!\n");
        return;
    }
    
    // Calculate the address in memory mapped configuration space.
    uint64_t config_addr = mcfg->reserved +
        (((uint64_t)bus << 20) | ((uint64_t)device << 15) | ((uint64_t)function << 12) | (offset & 0xFFF));

    // The vendor/device ID register is at offset 0 in the configuration space.
    volatile uint32_t *ptr = (volatile uint32_t *)config_addr;
    *ptr = value;
}


void parse_mcfg(mcfg_t *mcfg) {

    if (!mcfg) {
        if (debug_on) printf("[ACPI] MCFG table is NULL, cannot parse!\n");
        return;
    }
    // Calculate the number of allocation entries.
    // The total size of the table is in the header; subtract the size of the mcfg header
    // and then divide by the size of an allocation entry.
    uint32_t num_allocations = (mcfg->header.length - sizeof(mcfg_t)) / sizeof(mcfg_alloc_t);
    
    // The first allocation entry is immediately after the mcfg_t header.
    mcfg_alloc_t *alloc = (mcfg_alloc_t *)(mcfg + 1);

    // Loop over each allocation entry.
    for (uint32_t i = 0; i < num_allocations; i++) {
        printf("MCFG Allocation %d: Base Address = %x, Segment = %d, Bus Range = %d to %d\n",
               i, alloc[i].base_address, alloc[i].segment_group, alloc[i].start_bus, alloc[i].end_bus);

        // For each bus in the range specified by this allocation entry.
        for (uint8_t bus = alloc[i].start_bus; bus <= alloc[i].end_bus; bus++) {
            // For each device (0 to 31) and each function (0 to 7)
            for (uint8_t device = 0; device < 32; device++) {
                for (uint8_t function = 0; function < 8; function++) {
                    // Calculate the address in memory mapped configuration space.
                    uint64_t config_addr = alloc[i].base_address +
                        (((uint64_t)bus << 20) | ((uint64_t)device << 15) | ((uint64_t)function << 12));

                    // volatile uint32_t *ptr = (volatile uint32_t *)config_addr;   // Directly use physical address
                    uint32_t *ptr = (uint32_t *)phys_to_vir(config_addr);           // Map physical to virtual address
                    map_range(config_addr,(uint64_t)ptr, 0x10000000 /* 256MB for full ECAM */, PAGE_PRESENT | PAGE_WRITE  /* optionally PAGE_NOCACHE for MMIO */);
                    printf("MMCONFIG Address: %x\n", (uint64_t)ptr);
                    
                    // 0x00: Vendor ID + Device ID
                    uint32_t reg0 = ptr[0];
                    uint16_t vendor_id = reg0 & 0xFFFF;
                    uint16_t device_id = (reg0 >> 16) & 0xFFFF;
                    if (vendor_id == 0xFFFF || device_id == 0xFFFF) continue;       // No device present

                    // 0x08: Revision ID + Class Info
                    uint32_t reg2 = ptr[2];
                    uint8_t revision_id = reg2 & 0xFF;
                    uint8_t prog_if     = (reg2 >> 8) & 0xFF;
                    uint8_t subclass    = (reg2 >> 16) & 0xFF;
                    uint8_t class_code  = (reg2 >> 24) & 0xFF;

                    // 0x0C: BIST / Header Type / Latency Timer / Cache Line Size
                    uint32_t reg3 = ptr[3];
                    uint8_t cache_line_size = reg3 & 0xFF;
                    uint8_t latency_timer   = (reg3 >> 8) & 0xFF;
                    uint8_t header_type     = (reg3 >> 16) & 0x7F; // bit7 is multi-function flag
                    uint8_t bist            = (reg3 >> 24) & 0xFF;

                    // Find Base Address Registers (BAR0 to BAR5)
                    uint32_t bar[6];
                    bar[0] = pci_read_mmconfig(bus, device, function, 0x10);
                    bar[1] = pci_read_mmconfig(bus, device, function, 0x14);
                    bar[2] = pci_read_mmconfig(bus, device, function, 0x18);
                    bar[3] = pci_read_mmconfig(bus, device, function, 0x1C);
                    bar[4] = pci_read_mmconfig(bus, device, function, 0x20);
                    bar[5] = pci_read_mmconfig(bus, device, function, 0x24);

                    if(debug_on) printf(" PCI Device Found: Bus %d, Device %d, Function %d\n", bus, device, function);
                    if(debug_on) printf("  Vendor ID: %x, Device ID: %x\n", vendor_id, device_id);
                    if(debug_on) printf("  Class: %x, Subclass: %x, Prog IF: %x, Revision ID: %x\n",
                           class_code, subclass, prog_if, revision_id);
                    if(debug_on) printf("  Header Type: %x, BIST: %x, Cache Line Size: %x, Latency Timer: %x\n",
                           header_type, bist, cache_line_size, latency_timer);

                }
            }
        }
    }
}




