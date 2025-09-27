
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// Common PCI Vendor IDs
typedef enum {
    VENDOR_INTEL       = 0x8086,
    VENDOR_REALTEK     = 0x10EC,
    VENDOR_BROADCOM    = 0x14E4,
    VENDOR_AMD         = 0x1022,
    VENDOR_NVIDIA      = 0x10DE,
    VENDOR_VIA         = 0x1106,
    VENDOR_ATT         = 0x1259,
    VENDOR_QUALCOMM    = 0x17CB,
    VENDOR_AQUANTIA    = 0x1D6A,
    VENDOR_MELLANOX    = 0x15B3,
    VENDOR_VMWARE      = 0x15AD,
    VENDOR_VIRTIO      = 0x1AF4,
    VENDOR_QEMU        = 0x1234  // For some virtual NICs
} pci_vendor_id_t;




typedef enum {
    STANDARD_HEADER_SINGLE_FUNCTION = 0x0,   // Normal PCI/PCIe device (e.g., network card, VGA, sound card, etc.) 
    STANDARD_HEADER_MULTI_FUNCTION = 0x80,   // General device, multifunction

    PCI_TO_PCI_BRIDGE = 0x1,                 // Used when a PCI bus connects to another PCI/PCIe bus
    PCI_TO_PCI_BRIDGE_MULTIFUNC = 0x81,      // PCI-to-PCI bridge, multifunction

    CARD_BUS_BRIDGE = 0x2,                   // For legacy CardBus controllers (PCMCIA)
    CARD_BUS_BRIDGE_MULTIFUNC = 0x82         // CardBus bridge, multifunction
}HeaderType;


struct pci_device {
    uint8_t bus;                   // Bus number
    uint8_t device;                // Device number
    uint8_t function;              // Function number

    uint16_t vendor_id;            // Vendor ID
    uint16_t device_id;            // Device ID

    uint8_t class_code;            // Class code
    uint8_t subclass_code;         // Subclass code
    uint8_t prog_if;               // Programming interface

    uint8_t revision_id;           // Revision ID

    uint8_t bist;                  // Built-in self-test
    uint8_t cache_line_size;       // Cache line size
    uint8_t latency_timer;         // Latency timer
    uint8_t header_type;           // Header type

    uint32_t base_address_registers[6]; // Base address registers (BARs)
};
typedef struct pci_device pci_device_t;


extern pci_device_t *mass_storage_controllers;      // Array to store detected mass storage devices
extern int mass_storage_controllers_count;          // Counter for mass storage devices

extern pci_device_t *network_controllers;           // Array to store detected network controllers
extern int network_controllers_count;               // Counter for network controllers

extern pci_device_t *wireless_controllers;          // Array to store detected wireless controllers
extern int wireless_controllers_count;              // Counter for wireless controllers



uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);


bool pci_exists();
void pci_scan();




