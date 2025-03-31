
/*
    PCI (Peripheral Component Interconnect)

    ABAR : AHCI Base Memory Register

    https://wiki.osdev.org/PCI

    Build Date : 30-03-2025
    Developer Name : Bapon Kar
*/


#include "../driver/io/ports.h"
#include "../lib/stdio.h"
#include "../x86_64/timer/apic_timer.h"

#include "pci.h"



#define MAX_DRIVE_COUNT 256
#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC

#define PCI_VENDOR_ID_OFFSET  0x00
#define PCI_DEVICE_ID_OFFSET  0x02
#define PCI_CLASS_CODE_OFFSET 0x0B

pci_device_type_t hdd[MAX_DRIVE_COUNT]; // AHCI SATA HDD
pci_device_type_t ssd[MAX_DRIVE_COUNT];
pci_device_type_t usbd[MAX_DRIVE_COUNT];


/*
 ____________________________________________________________________________________________________
| Bit 31	   |   Bits 30-24  |  Bits 23-16 |   Bits 15-11	  |  Bits 10-8	     |  Bits 7-0         |
|--------------|---------------|-------------|----------------|------------------|-------------------|
| Enable Bit   | Reserved	   | Bus Number	 |  Device Number |	 Function Number |	Register Offset1 |
|______________|_______________|_____________|________________|__________________|___________________|
*/

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(CONFIG_ADDRESS, address);  // Write CONFIG_ADDRESS
    return inl(CONFIG_DATA);        // Read CONFIG_DATA
}

void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(CONFIG_ADDRESS, address);  // Write CONFIG_ADDRESS
    outl(CONFIG_DATA, value);       // Write CONFIG_DATA
}



void pci_scan() {
    bool gpu_found = false;
    bool storage_found = false;
    bool network_found = false;

    printf("\n[INFO] Scanning PCI devices... ");

    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {

                uint32_t vendor_device = pci_read(bus, device, function, PCI_VENDOR_ID_OFFSET);
                uint16_t vendor_id = vendor_device & 0xFFFF;
                uint16_t device_id = vendor_device >> 16;

                // Check if the device exists
                if (vendor_id == 0xFFFF) {
                    continue; // Invalid device
                }

                // Read the class and subclass codes
                uint32_t class_code = pci_read(bus, device, function, PCI_CLASS_CODE_OFFSET);
                PCIClass class = (class_code >> 24) & 0xFF;
                uint8_t subclass = (class_code >> 16) & 0xFF;

                printf("\n[PCI Device] Bus %d, Device %d, Function %d\n", bus, device, function);
                printf("  - Vendor ID: %x, Device ID: %x\n", vendor_id, device_id);
                printf("  - Class: %x, Subclass: %x\n", class, subclass);

                switch (class) {
                    case PCI_CLASS_MASS_STORAGE_CONTROLLER:
                        if (!storage_found) {
                            printf("  - Type: Storage Controller\n");
                            switch (subclass) {
                                case PCI_SUBCLASS_SERIAL_ATA:  
                                    printf("    - Subtype: ATA (AHCI) Controller\n");
                                    uint32_t bar_low = pci_read(bus, device, function, 0x24);
                                    printf("    - ABAR: %x\n", bar_low);
                                    uint32_t bar_high = 0;
                                    uint8_t bar_type = bar_low & 0x7;
                                    if (bar_type == 0x4) {
                                        bar_high = pci_read(bus, device, function, 0x28);
                                    }
                                    // ahci_ctrl.abar = ((uint64_t)bar_high << 32) | (bar_low & ~0xFULL);
                                    // ahci_ctrl.bus = bus;
                                    // ahci_ctrl.device = device;
                                    // ahci_ctrl.function = function;
                                    break;
                                case PCI_SUBCLASS_SCSI: 
                                    printf("    - Subtype: NVMe Controller\n"); 
                                    break;
                                default:                
                                    printf("    - Subtype: Other Storage Controller\n"); 
                                    break;
                            }
                            storage_found = true;
                        }
                        break;

                    case PCI_CLASS_NETWORK_CONTROLLER:
                        if (!network_found) {
                            printf("  - Type: Network Controller\n");
                            network_found = true;
                        }
                        break;

                    case PCI_CLASS_DISPLAY_CONTROLLER:
                        if (!gpu_found) {
                            printf("  - Type: Display Controller (GPU)\n");
                            gpu_found = true;
                        }
                        break;

                    default:
                        continue;
                        break;
                }

                if (gpu_found && storage_found && network_found) {
                    printf("\n[INFO] Essential components found. Stopping PCI scan.\n");
                    return;
                }
            }
        }
    }
}


