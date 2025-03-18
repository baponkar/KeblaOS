
/*
AHCI (Advance Host Controller Interface)

References:
    https://wiki.osdev.org/AHCI
*/

#include "../lib/stdio.h"
#include "../driver/io/ports.h"

#include "ahci.h"

#define AHCI_CAP 0x00
#define AHCI_PI  0x0C
#define AHCI_VS  0x10
#define AHCI_CMD 0x18
#define AHCI_IS  0x08

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}


void detect_ahci() {
    bool found = false;
    for (uint8_t bus = 0; bus < 256 && !found; bus++) {
        for (uint8_t slot = 0; slot < 32 && !found; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t vendor_device = pci_read(bus, slot, func, 0x00);
                uint16_t vendor_id = vendor_device & 0xFFFF;
                if (vendor_id == 0xFFFF)
                    continue; // Invalid device

                uint32_t class_rev = pci_read(bus, slot, func, 0x08);
                uint8_t class_code = (class_rev >> 24) & 0xFF;
                uint8_t subclass_code = (class_rev >> 16) & 0xFF;

                if (class_code == 0x01 && subclass_code == 0x06) { // AHCI Controller
                    printf("AHCI Controller found at Bus %d, Slot %d, Function %d\n", bus, slot, func);
                    uint32_t bar5 = pci_read(bus, slot, func, 0x24);
                    if (bar5 & 1) {
                        printf("AHCI BAR5 is in I/O space (unsupported)\n");
                    } else {
                        uint32_t ahci_base = bar5 & ~0xF;
                        printf("AHCI Base Address: %x\n", ahci_base);
                        init_ahci(ahci_base);
                    }
                    found = true;
                    break;
                }
            }
        }
    }
}



void init_ahci(uint32_t ahci_base) {
    uint32_t cap = inl(ahci_base + AHCI_CAP);
    uint32_t pi = inl(ahci_base + AHCI_PI);

    printf("AHCI Capabilities: %x\n", cap);
    printf("AHCI Ports Implemented: %x\n", pi);

    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            ahci_port_t *port = (ahci_port_t *)(ahci_base + 0x100 + i * 0x80);
            uint32_t ssts = port->ssts;

            if ((ssts & 0x0F) == 3) { // Device detected and Phy communication established
                printf("Port %d: Drive detected\n", i);
                // Configure and use this port
                break;
            }
        }
    }
}

