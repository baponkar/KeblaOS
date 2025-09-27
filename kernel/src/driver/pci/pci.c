
/*
    PCI (Peripheral Component Interconnect)

    ABAR : AHCI Base Memory Register
    Prog IF : Programming Interface Byte

    https://wiki.osdev.org/PCI
    https://www.pcilookup.com/
    https://pcisig.com/membership/member-companies

    Build Date : 30-03-2025
    Developer Name : Bapon Kar
*/


#include "../../sys/controllers/controllers.h"

#include "../../driver/disk/ahci/ahci.h"
#include "../../driver/io/ports.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../memory/kheap.h"

#include "pci.h"

extern bool debug_on;

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC

#define DEVICE_VENDOR_OFFSET 0x0
#define STATUS_COMMAND_OFFSET 0x4

#define CLASS_SUBCLASS_PROG_REV_OFFSET 0x8
#define BIST_HEADER_LATENCY_CACHE_LINE_OFFSET 0xC

#define DEVICE_ID_OFFSET 0x2
#define VENDOR_ID_OFFSET 0x0


extern pci_device_t* pci_devices;                  
extern int pci_devices_count;


/*
PCI CONFIG ADDRESS
 __________________________________________________________________________________________________________________
| Bit 31	   |   Bits 30-24  |  Bits 23-16 |   Bits 15-11	  |  Bits 10-8	     |  Bits 7-2         |  Bits 1-0   |
|--------------|---------------|-------------|----------------|------------------|-------------------|-------------|
| Enable Bit   | Reserved	   | Bus Number	 |  Device Number |	 Function Number |	Register Offset1 |  00         |
|______________|_______________|_____________|________________|__________________|___________________|_____________|
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


bool pci_exists() {
    // Try reading Vendor ID of bus 0, device 0, function 0
    uint32_t value = pci_read(0, 0, 0, 0);
    uint16_t vendor_id = value & 0xFFFF;
    return (vendor_id != 0xFFFF);
}


pci_device_t pci_device_detect(uint8_t bus, uint8_t device, uint8_t function) {

    // Initialize with zero
    volatile pci_device_t pci_device = {0};

    // Read the device and vendor ID
    uint32_t _dev_vend = pci_read(bus, device, function, DEVICE_VENDOR_OFFSET);
    pci_device.device_id = _dev_vend >> 16;       // upper 16 bit
    pci_device.vendor_id = _dev_vend & 0xFFFF;    // lower 16 bit

    // Read the status and command register
    uint32_t class_subclass_prog_if = pci_read(bus, device, function, CLASS_SUBCLASS_PROG_REV_OFFSET);
    pci_device.class_code = (class_subclass_prog_if >> 24) & 0xFF;              // upper 8 bit
    pci_device.subclass_code = (class_subclass_prog_if >> 16) & 0xFF;           // middle 8 bit
    pci_device.prog_if = (class_subclass_prog_if >> 8) & 0xFF;                  // lower 8 bit
    pci_device.revision_id = class_subclass_prog_if & 0xFF;                     // lower 8 bit

    // Read the BIST, header type, latency timer, and cache line size
    uint32_t bist_header_latency_cache_line = pci_read(bus, device, function, BIST_HEADER_LATENCY_CACHE_LINE_OFFSET);
    pci_device.bist = (bist_header_latency_cache_line >> 24) & 0xFF;           // upper 8 bit
    pci_device.header_type = (bist_header_latency_cache_line >> 16) & 0xFF;    // middle 8 bit
    pci_device.latency_timer = (bist_header_latency_cache_line >> 8) & 0xFF;   // lower 8 bit
    pci_device.cache_line_size = bist_header_latency_cache_line & 0xFF;        // lower 8 bit

    // Set BAR
    if(pci_device.header_type ==  STANDARD_HEADER_SINGLE_FUNCTION){
        // if(debug_on) printf("Standard Header Single Function found: %x\n", pci_device.header_type);
        // read the BAR0 to BAR5 (Base Address Register 0 to 5)
        for(int i = 0; i < 6; i++){
            if(i == 5) {    // This is a pointer to the AHCI controller's memory address
                pci_device.base_address_registers[i] = pci_read(bus, device, function, 0x10 + (i * 4)) & 0xFFFFFFF0;
            }else{
                pci_device.base_address_registers[i] = pci_read(bus, device, function, 0x10 + (i * 4));
            }
        }
    }else if(pci_device.header_type ==  STANDARD_HEADER_MULTI_FUNCTION){
        // if(debug_on) printf("STANDARD_HEADER_MULTI_FUNCTION: %x\n", pci_device.header_type);
        // read the BAR0 to BAR5 (Base Address Register 0 to 5)
        for(int i = 0; i < 6; i++){
            if(i == 5) {    // This is a pointer to the AHCI controller's memory address
                pci_device.base_address_registers[i] = pci_read(bus, device, function, 0x10 + (i * 4)) & 0xFFFFFFF0;
            }else{
                pci_device.base_address_registers[i] = pci_read(bus, device, function, 0x10 + (i * 4));
            }
        }
    }else if(pci_device.header_type ==  PCI_TO_PCI_BRIDGE){
        if(debug_on) printf("PCI TO PCI BRIDGE Single function Header found: %x\n", pci_device.header_type);
    }else if(pci_device.header_type == PCI_TO_PCI_BRIDGE_MULTIFUNC){
        if(debug_on) printf("PCI_TO_PCI_BRIDGE_MULTIFUNC: %x\n", pci_device.header_type);
    }else if(pci_device.header_type ==  CARD_BUS_BRIDGE){
        if(debug_on) printf("Card Bus Bridge Header found: %x\n", pci_device.header_type);
    }else if(pci_device.header_type ==  CARD_BUS_BRIDGE_MULTIFUNC){
        if(debug_on) printf("Multi Function Header Found: %x\n", pci_device.header_type);
    }

    if(debug_on) printf(" Device ID: %x, Vendor ID: %x, Class: %x, Subclass: %x\n",
        pci_device.device_id, pci_device.vendor_id, pci_device.class_code, pci_device.subclass_code);

    return pci_device;
}





void pci_scan() {

    if(debug_on) printf("\n[PCI] Scanning PCI devices......\n");

    if(!pci_exists()){
        printf(" PCI: No PCI bus found!\n");
        return;
    }

    for (uint32_t index = 0; index < (256 * 32 * 8); index++) {
        uint8_t bus = (index >> 8) & 0xFF;
        uint8_t device = (index >> 3) & 0x1F;
        uint8_t function = index & 0x07;

        // read the device and  vendor id
        uint32_t _dev_vend = pci_read(bus, device, function, DEVICE_VENDOR_OFFSET);
        uint16_t device_id = _dev_vend >> 16;       // upper 16 bit
        uint16_t vendor_id = _dev_vend & 0xFFFF;    // lower 16 bi

        // Check if the device exists
        if (device_id == 0xFFFF || device_id == 0x0000 || vendor_id == 0xFFFF || vendor_id == 0x0000) {
            continue; // Invalid device
        }
        
        pci_device_t found_device = pci_device_detect(bus, device, function);
        pci_devices[pci_devices_count++] = found_device; // Store the detected PCI device
    }

    if(debug_on) printf("[PCI] PCI Scan Completed..........\n\n");
}










