
/*
    PCI (Peripheral Component Interconnect)

    ABAR : AHCI Base Memory Register

    https://wiki.osdev.org/PCI
    
*/


#include "../driver/io/ports.h"
#include "../lib/stdio.h"
#include "../ahci/ahci.h"

#include "pci.h"

typedef enum {
    PCI_CLASS_UNCLASSIFIED     = 0x00, // Non-VGA device
    PCI_CLASS_STORAGE          = 0x01, // Mass Storage Controllers
    PCI_CLASS_NETWORK          = 0x02, // Network Controllers
    PCI_CLASS_DISPLAY          = 0x03, // Display Controllers (e.g., VGA, GPUs)
    PCI_CLASS_MULTIMEDIA       = 0x04, // Multimedia Devices (e.g., Sound Cards)
    PCI_CLASS_MEMORY           = 0x05, // Memory Controllers
    PCI_CLASS_BRIDGE           = 0x06, // Bridge Devices (e.g., PCI-PCI Bridge)
    PCI_CLASS_SIMPLE_COMM      = 0x07, // Simple Communication Controllers (e.g., Serial, Parallel Ports)
    PCI_CLASS_BASE_SYSTEM      = 0x08, // Base System Peripherals
    PCI_CLASS_INPUT            = 0x09, // Input Devices (e.g., Keyboards, Mice)
    PCI_CLASS_DOCKING          = 0x0A, // Docking Stations
    PCI_CLASS_PROCESSOR        = 0x0B, // Processors (e.g., Embedded CPUs)
    PCI_CLASS_SERIAL_BUS       = 0x0C, // Serial Bus Controllers (e.g., USB, FireWire)
    PCI_CLASS_WIRELESS         = 0x0D, // Wireless Controllers
    PCI_CLASS_INTELLIGENT_IO   = 0x0E, // Intelligent I/O Controllers
    PCI_CLASS_SATELLITE_COMM   = 0x0F, // Satellite Communication Controllers
    PCI_CLASS_ENCRYPTION       = 0x10, // Encryption/Decryption Controllers
    PCI_CLASS_SIGNAL_PROCESS   = 0x11, // Signal Processing Controllers
    PCI_CLASS_PROCESSING_ACCEL = 0x12, // Processing Accelerators (e.g., AI, Tensor Cores)
    PCI_CLASS_NON_ESSENTIAL    = 0x13, // Non-Essential Instrumentation
    
    PCI_CLASS_COPROCESSOR      = 0x40, // Co-Processors
    PCI_CLASS_UNASSIGNED       = 0xFF  // Device does not fit any known class
} PCIClass;


typedef enum {
    PCI_SUBCLASS_SCSI          = 0x00,
    PCI_SUBCLASS_IDE           = 0x01,
    PCI_SUBCLASS_FLOPPY        = 0x02,
    PCI_SUBCLASS_IPI           = 0x03,
    PCI_SUBCLASS_RAID          = 0x04,
    PCI_SUBCLASS_ATA           = 0x05, // Includes AHCI
    PCI_SUBCLASS_SAS           = 0x06,
    PCI_SUBCLASS_NVME          = 0x07,
} PCIStorageSubclass;


#define PCI_VENDOR_ID_OFFSET  0x00
#define PCI_DEVICE_ID_OFFSET  0x02
#define PCI_CLASS_CODE_OFFSET 0x0B


extern ahci_controller_t ahci_ctrl; // Defined in ahci

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(0xCF8, address); // Write CONFIG_ADDRESS
    return inl(0xCFC);    // Read CONFIG_DATA
}

void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(0xCF8, address); // Write CONFIG_ADDRESS
    outl(0xCFC, value);   // Write CONFIG_DATA
}



void pci_scan() {
    bool gpu_found = false;
    bool storage_found = false;
    bool network_found = false;

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
                    case PCI_CLASS_STORAGE:
                        if (!storage_found) {
                            printf("  - Type: Storage Controller\n");
                            switch (subclass) {
                                case PCI_SUBCLASS_SAS:  
                                    printf("    - Subtype: ATA (AHCI) Controller\n");
                                    // Read BAR5 (offset 0x24) to get ABAR
                                    uint32_t bar_low = pci_read(bus, device, function, 0x24);
                                    printf("    - ABAR: %x\n", bar_low);
                                    uint32_t bar_high = 0;
                                    uint8_t bar_type = bar_low & 0x7;
                                    if (bar_type == 0x4) { // 64-bit memory space
                                        bar_high = pci_read(bus, device, function, 0x28);
                                    }
                                    ahci_ctrl.abar = ((uint64_t)bar_high << 32) | (bar_low & ~0xFULL);
                                    ahci_ctrl.bus = bus;
                                    ahci_ctrl.device = device;
                                    ahci_ctrl.function = function;

                                    break;
                                case PCI_SUBCLASS_NVME: 
                                    printf("    - Subtype: NVMe Controller\n"); 
                                    break;
                                default:                
                                    printf("    - Subtype: Other Storage Controller\n"); 
                                    break;
                            }
                            storage_found = true;
                        }
                        break;

                    case PCI_CLASS_NETWORK:
                        if (!network_found) {
                            printf("  - Type: Network Controller\n");
                            network_found = true;
                        }
                        break;

                    case PCI_CLASS_DISPLAY:
                        if (!gpu_found) {
                            printf("  - Type: Display Controller (GPU)\n");
                            gpu_found = true;
                        }
                        break;

                    default:
                        continue;
                        break;
                }

                // Stop scanning if all essential devices are found
                if (gpu_found && storage_found && network_found) {
                    printf("\n[INFO] Essential components found. Stopping PCI scan.\n");
                    return;
                }
            }
        }
    }
}






