/*

*/

#include "../driver/io/ports.h"
#include "../lib/stdio.h"

#include "pci.h"

uint32_t pci_read1(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(0xCF8, address); // Write CONFIG_ADDRESS
    return inl(0xCFC);    // Read CONFIG_DATA
}

void pci_write1(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(0xCF8, address); // Write CONFIG_ADDRESS
    outl(0xCFC, value);   // Write CONFIG_DATA
}



#define PCI_VENDOR_ID_OFFSET 0x00
#define PCI_DEVICE_ID_OFFSET 0x02
#define PCI_CLASS_CODE_OFFSET 0x0B

void pci_scan() {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint32_t vendor_device = pci_read1(bus, device, function, PCI_VENDOR_ID_OFFSET);
                uint16_t vendor_id = vendor_device & 0xFFFF;
                uint16_t device_id = vendor_device >> 16;

                // Check if the device exists
                if (vendor_id == 0xFFFF) {
                    continue; // Invalid device
                }

                // Read the class code
                uint32_t class_code = pci_read1(bus, device, function, PCI_CLASS_CODE_OFFSET);
                uint8_t class = (class_code >> 24) & 0xFF;

                // Check if this is a VGA compatible controller
                if (class == 0x03) {
                    printf("GPU Found: Bus %d, Device %d, Function %d\n", bus, device, function);
                    printf("Vendor ID: %x, Device ID: %x\n", vendor_id, device_id);
                }
            }
        }
    }
}






