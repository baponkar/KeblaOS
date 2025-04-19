
/*
    PCI (Peripheral Component Interconnect)

    ABAR : AHCI Base Memory Register
    Prog IF : Programming Interface Byte

    https://wiki.osdev.org/PCI
    https://www.pcilookup.com/

    Build Date : 30-03-2025
    Developer Name : Bapon Kar
*/

#include "../ahci/ahci.h"
#include "../driver/io/ports.h"
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../memory/kheap.h"

#include "pci.h"



#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC

#define DEVICE_VENDOR_OFFSET 0x0
#define STATUS_COMMAND_OFFSET 0x4
#define CLASS_SUBCLASS_PROG_REV_OFFSET 0x8
#define BIST_HEADER_LATENCY_CACHE_LINE_OFFSET 0xC


ahci_controller_t sata_disk;
ahci_controller_t network_controller;


/*
PCI CONFIG
 __________________________________________________________________________________________________________________
| Bit 31	   |   Bits 30-24  |  Bits 23-16 |   Bits 15-11	  |  Bits 10-8	     |  Bits 7-2         |  Bits 1-0   |
|--------------|---------------|-------------|----------------|------------------|-------------------|-------------|
| Enable Bit   | Reserved	   | Bus Number	 |  Device Number |	 Function Number |	Register Offset1 |  00         |
|______________|_______________|_____________|________________|__________________|___________________|_____________|
*/

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(CONFIG_ADDRESS, address);  // Write CONFIG_ADDRESS
    return inl(CONFIG_DATA);   // Read CONFIG_DATA     
}

void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(CONFIG_ADDRESS, address);  // Write CONFIG_ADDRESS
    outl(CONFIG_DATA, value);       // Write CONFIG_DATA
}


int detect_disk_controller(uint8_t bus, uint8_t device, uint8_t function,
    uint8_t class, uint8_t subclass, uint8_t prog_if, uint8_t revision) {

    if(class == PCI_CLASS_MASS_STORAGE_CONTROLLER){
        if(subclass == PCI_SUBCLASS_SERIAL_ATA){
            if(prog_if == 0x1){
                // Setting abar
                uint32_t bar_low = pci_read(bus, device, function, 0x24); // Read BAR5 low 32 bits
                uint32_t bar_high = 0;  
                if ((bar_low & 0x7) == 0x4) {                            // Check if it's a 64-bit BAR
                    bar_high = pci_read(bus, device, function, 0x28);    // Read BAR5 high 32 bits
                }

                // Adding found disk into sata_disks array
                sata_disk.bus = bus;
                sata_disk.device = device;
                sata_disk.function = function;
                sata_disk.abar = ((uint64_t)bar_high << 32) | ((uint64_t)bar_low & ~0xFULL);
                sata_disk.initialized = false; // This sata drive will initialized by ahci

                printf(" [-] PCI: Detected SATA Disk at %d:%d.%d - ", bus, device, function);
                printf(" Class: %d, Subclass: %d, Prog IF: %d, Revision: %d\n",
                       class, subclass, prog_if, revision);
                return 1;
            }
        }

        if(subclass == PCI_SUBCLASS_ATA) {
            // Setting abar
            uint32_t bar_low = pci_read(bus, device, function, 0x24); // Read BAR5 low 32 bits
            uint32_t bar_high = 0;  
            if ((bar_low & 0x7) == 0x4) {                            // Check if it's a 64-bit BAR
                bar_high = pci_read(bus, device, function, 0x28);    // Read BAR5 high 32 bits
            }

            // Adding found disk into sata_disks array
            sata_disk.bus = bus;
            sata_disk.device = device;
            sata_disk.function = function;
            sata_disk.abar = ((uint64_t)bar_high << 32) | ((uint64_t)bar_low & ~0xFULL);
            sata_disk.initialized = false; // This sata drive will initialized by ahci

            printf(" [-] PCI: Detected ATA Disk at %d:%d.%d - ", bus, device, function);
            printf(" Class: %d, Subclass: %d, Prog IF: %d, Revision: %d\n",
                   class, subclass, prog_if, revision);
            return 1;
        }

        if(subclass == PCI_SUBCLASS_IDE){
            // Setting abar
            uint32_t bar_low = pci_read(bus, device, function, 0x24); // Read BAR5 low 32 bits
            uint32_t bar_high = 0;  
            if ((bar_low & 0x7) == 0x4) {                            // Check if it's a 64-bit BAR
                bar_high = pci_read(bus, device, function, 0x28);    // Read BAR5 high 32 bits
            }

            // Adding found disk into sata_disks array
            sata_disk.bus = bus;
            sata_disk.device = device;
            sata_disk.function = function;
            sata_disk.abar = ((uint64_t)bar_high << 32) | ((uint64_t)bar_low & ~0xFULL);
            sata_disk.initialized = false; // This sata drive will initialized by ahci

            printf(" [-] PCI: Detected IDE Disk at %d:%d.%d - ", bus, device, function);
            printf("  Class: %d, Subclass: %d, Prog IF: %d, Revision: %d\n",
                   class, subclass, prog_if, revision);
            return 1;
        }
    }
    // Not a SATA disk
    return 0;
}



int detect_network_controller(uint8_t bus, uint8_t device, uint8_t function,
    uint8_t class, uint8_t subclass, uint8_t prog_if, uint8_t revision) {

    if (class == PCI_CLASS_NETWORK_CONTROLLER) {
        printf(" [-] PCI: Detected Network Controller at %d:%d.%d - ", bus, device, function);

        switch (subclass) {
            case PCI_SUBCLASS_ETHERNET:
                printf("  Ethernet Controller\n");
                // Initialize Ethernet driver here
                break;
            case PCI_SUBCLASS_TOKEN_RING:
                printf("  Token Ring Controller\n");
                break;
            case PCI_SUBCLASS_FDDI:
                printf("  FDDI Controller\n");
                break;
            case PCI_SUBCLASS_ATM:
                printf("  ATM Controller\n");
                break;
            case PCI_SUBCLASS_ISDN:
                printf("  ISDN Controller\n");
                break;
            case PCI_SUBCLASS_WORLD_FIP:
                printf("  World Fip Controller\n");
                break;
            case PCI_SUBCLASS_PIC_MG:
                printf("  PIC fib Controller\n");
                break;
            case PCI_SUBCLASS_FABRIC:
                printf("  Fabric  Controller\n");
                break;
            default:
                printf("  Unknown Network Controller (Subclass: %x, Prog IF: %x, Revision: %x)\n",
                       subclass, prog_if, revision);
                break;
        }
        return 1; // Successfully detected a network controller
    }

    return 0; // Not a network controller
}


int detect_wireless_controller(uint8_t bus, uint8_t device, uint8_t function,
    uint8_t class, uint8_t subclass, uint8_t prog_if, uint8_t revision) {

    if (class == PCI_CLASS_WIRELESS_CONTROLLER) {
        printf(" [-]PCI: Detected Wireless Network Controller at %d:%d.%d - ", bus, device, function);
    
        switch (subclass) {
            case PCI_SUBCLASS_IRDA:
                printf("  IRDA Wireless found\n");
                break;
    
            case PCI_SUBCLASS_IR:
                printf("  Infrared (IR) Wireless found\n");
                break;
    
            case PCI_SUBCLASS_RF:
                printf("  Radio Frequency (RF) Wireless found\n");
                break;
    
            case PCI_SUBCLASS_BLUETOOTH:
                printf("  Bluetooth Wireless found\n");
                break;
    
            case PCI_SUBCLASS_BROADBAND:
                printf("  Broadband Wireless found\n");
                break;
    
            case PCI_SUBCLASS_ETHERNET_802_1_a:
                printf("  Ethernet 802.1a Wireless found\n");
                break;
    
            case PCI_SUBCLASS_ETHERNET_802_1_b:
                printf("  Ethernet 802.1b Wireless found\n");
                break;
    
            default:
                printf("  Unknown Wireless Network Controller (Subclass: %x, Prog IF: %x, Revision: %x)\n",
                        subclass, prog_if, revision);
                break;
        }
    }        
}


void pci_scan() {

    printf("[INFO] Scanning PCI devices... \n");

    for (uint32_t index = 0; index < (256 * 32 * 8); index++) {
        uint8_t bus = (index >> 8) & 0xFF;
        uint8_t device = (index >> 3) & 0x1F;
        uint8_t function = index & 0x07;

        // read the device and  vendor id
        uint32_t _dev_vend = pci_read(bus, device, function, DEVICE_VENDOR_OFFSET);
        uint16_t device_id = _dev_vend >> 16;       // upper 16 bit
        uint16_t vendor_id = _dev_vend & 0xFFFF;    // lower 16 bi

        // Check if the device exists
        if (device_id == 0xFFFF | vendor_id == 0xFFFF) {
            continue; // Invalid device
        }
        
        // Read the status and command
        uint32_t _status_command = pci_read(bus, device, function, STATUS_COMMAND_OFFSET);
        uint16_t status = _status_command >> 16;       // upper 16 bit
        uint16_t command = _status_command & 0xFFFF;   // lower 16 bit
        
        // Read the class and subclass codes
        uint32_t _class_code = pci_read(bus, device, function, CLASS_SUBCLASS_PROG_REV_OFFSET);
        uint8_t class = (_class_code >> 24) & 0xFF;     // upper 8 bit
        uint8_t subclass = (_class_code >> 16) & 0xFF;  // 23-16 th bit
        uint8_t prog_if = (_class_code >> 8) & 0xFF;    // 15-8 th bit
        uint8_t revision = (_class_code >> 0) & 0xFF;   // lower 8 bit

        // Read bist, header type, latency timer, cache line size
        uint32_t _bist = pci_read(bus, device, function, BIST_HEADER_LATENCY_CACHE_LINE_OFFSET);
        uint8_t bist = (_bist >> 24) & 0xFF;            // upper 8 bit
        uint8_t header_type = (_bist >> 16) & 0xFF;     // 23-16 th bit
        uint8_t latency_timer = (_bist >> 8) & 0xFF;    // 15-8 th bit
        uint8_t cache_line_size = (_bist >> 0) & 0xFF;  // lower 8 bit

        // printf("bus: %d - device: %d - function: %d\n",bus, device, function );
        /*printf("Header Type: %x => Device ID: %x, Vendor ID: %x, class: %x, subclass: %x, prog_if: %x\n",
            header_type, device_id, vendor_id, class, subclass, prog_if);*/

        // Detecting SATA Disk and storing into sata_disks array
        int found_sata = detect_disk_controller(bus, device, function, class, subclass, prog_if, revision);
        // Detecting Network Controller
        int found_network = detect_network_controller(bus, device, function, class, subclass, prog_if, revision);
        // Detecting Wireless controller
        int found_wireless = detect_wireless_controller(bus, device, function, class, subclass, prog_if, revision);
    }
    printf("[INFO] PCI Scan Completed\n");
}



