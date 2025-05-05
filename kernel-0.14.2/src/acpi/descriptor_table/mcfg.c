
/*
MCFG (Memory-Mapped Configuration Table)

https://wiki.osdev.org/PCI_Express
*/

#include "../../lib/stdio.h"

#include "mcfg.h"


mcfg_t *mcfg; // Defined in acpi.c



void parse_mcfg(mcfg_t *mcfg) {
    // Calculate the number of allocation entries.
    // The total size of the table is in the header; subtract the size of the mcfg header
    // and then divide by the size of an allocation entry.
    uint32_t num_allocations = (mcfg->header.length - sizeof(mcfg_t)) / sizeof(mcfg_alloc_t);
    
    // The first allocation entry is immediately after the mcfg_t header.
    mcfg_alloc_t *alloc = (mcfg_alloc_t *)(mcfg + 1);

    // Loop over each allocation entry.
    for (uint32_t i = 0; i < num_allocations; i++) {
        printf("MCFG Allocation %d: Base Address = %x, Segment = %d, Bus Range = %d to %d\n",
               i, alloc[i].base_address, alloc[i].segment_group,
               alloc[i].start_bus, alloc[i].end_bus);

        // For each bus in the range specified by this allocation entry.
        for (uint8_t bus = alloc[i].start_bus; bus <= alloc[i].end_bus; bus++) {
            // For each device (0 to 31) and each function (0 to 7)
            for (uint8_t device = 0; device < 32; device++) {
                for (uint8_t function = 0; function < 8; function++) {
                    // Calculate the address in memory mapped configuration space.
                    uint64_t config_addr = alloc[i].base_address +
                        (((uint64_t)bus << 20) | ((uint64_t)device << 15) | ((uint64_t)function << 12));

                    // The vendor/device ID register is at offset 0 in the configuration space.
                    volatile uint32_t *ptr = (volatile uint32_t *)config_addr;
                    uint32_t vendor_device = *ptr;
                    uint16_t vendor_id = vendor_device & 0xFFFF;
                    uint16_t device_id = (vendor_device >> 16) & 0xFFFF;

                    // If vendor_id is 0xFFFF, then no device is present.
                    if (vendor_id == 0xFFFF)
                        continue;

                    // Otherwise, print the device information.
                    printf("PCIe Device Found: Bus %d, Device %d, Function %d - Vendor ID: %x, Device ID: %x\n",
                           bus, device, function, vendor_id, device_id);

                    // Optionally, you can read more configuration registers here,
                    // such as the class code, subclass, etc.
                }
            }
        }
    }
}




