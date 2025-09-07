
/*
    PCI (Peripheral Component Interconnect)

    ABAR : AHCI Base Memory Register
    Prog IF : Programming Interface Byte

    https://wiki.osdev.org/PCI
    https://www.pcilookup.com/

    Build Date : 30-03-2025
    Developer Name : Bapon Kar
*/

#include "../../driver/disk/ahci/ahci.h"
#include "../../driver/io/ports.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../memory/kheap.h"

#include "pci.h"



#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC

#define DEVICE_VENDOR_OFFSET 0x0
#define STATUS_COMMAND_OFFSET 0x4

#define CLASS_SUBCLASS_PROG_REV_OFFSET 0x8
#define BIST_HEADER_LATENCY_CACHE_LINE_OFFSET 0xC

#define DEVICE_ID_OFFSET 0x2
#define VENDOR_ID_OFFSET 0x0


pci_device_t* mass_storage_controllers;  // Array to store detected mass storage devices
size_t mass_storage_count = 0;           // Counter for mass storage devices

pci_device_t *network_controllers;       // Array to store detected network controllers
size_t network_controller_count = 0;     // Counter for network controllers

pci_device_t *wireless_controllers;      // Array to store detected wireless controllers
size_t wireless_controller_count = 0;    // Counter for wireless controllers



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
    return inl(CONFIG_DATA);        // Read CONFIG_DATA     
}

void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(CONFIG_ADDRESS, address);  // Write CONFIG_ADDRESS
    outl(CONFIG_DATA, value);       // Write CONFIG_DATA
}

pci_device_t pci_device_detect(uint8_t bus, uint8_t device, uint8_t function) {

    // Initialize with zero
    volatile pci_device_t pci_device;
    pci_device.bus = 0;
    pci_device.device = 0;
    pci_device.function = 0;
    pci_device.vendor_id = 0;
    pci_device.device_id = 0;
    pci_device.class_code = 0;
    pci_device.subclass_code = 0;
    pci_device.prog_if = 0;
    pci_device.revision_id = 0;
    pci_device.bist = 0;
    pci_device.cache_line_size = 0;
    pci_device.latency_timer = 0;
    pci_device.header_type = 0;
    pci_device.base_address_registers[0] = 0;
    pci_device.base_address_registers[1] = 0;
    pci_device.base_address_registers[2] = 0;
    pci_device.base_address_registers[3] = 0;
    pci_device.base_address_registers[4] = 0;
    pci_device.base_address_registers[5] = 0;


    // Read the device and vendor ID
    uint32_t _dev_vend = pci_read(bus, device, function, DEVICE_VENDOR_OFFSET);
    uint16_t device_id = _dev_vend >> 16;       // upper 16 bit
    uint16_t vendor_id = _dev_vend & 0xFFFF;    // lower 16 bit

    pci_device.device_id = device_id;
    pci_device.vendor_id = vendor_id;

    // Read the status and command register
    uint32_t class_subclass_prog_if = pci_read(bus, device, function, CLASS_SUBCLASS_PROG_REV_OFFSET);
    uint8_t class = (class_subclass_prog_if >> 24) & 0xFF;       // upper 8 bit
    uint8_t subclass = (class_subclass_prog_if >> 16) & 0xFF;    // middle 8 bit
    uint8_t prog_if = (class_subclass_prog_if >> 8) & 0xFF;      // lower 8 bit

    pci_device.class_code = class;
    pci_device.subclass_code = subclass;
    pci_device.prog_if = prog_if;
    pci_device.revision_id = class_subclass_prog_if & 0xFF;      // lower 8 bit

    // Read the BIST, header type, latency timer, and cache line size
    uint32_t bist_header_latency_cache_line = pci_read(bus, device, function, BIST_HEADER_LATENCY_CACHE_LINE_OFFSET);
    uint8_t bist = (bist_header_latency_cache_line >> 24) & 0xFF;           // upper 8 bit
    uint8_t header_type = (bist_header_latency_cache_line >> 16) & 0xFF;    // middle 8 bit
    uint8_t latency_timer = (bist_header_latency_cache_line >> 8) & 0xFF;   // lower 8 bit
    uint8_t cache_line_size = bist_header_latency_cache_line & 0xFF;        // lower 8 bit

    pci_device.bist = bist;
    pci_device.header_type = header_type;
    pci_device.latency_timer = latency_timer;
    pci_device.cache_line_size = cache_line_size;

    // read the BAR0 to BAR5 (Base Address Register 0 to 5)
    uint32_t bar0 = pci_read(bus, device, function, 0x10); // Read BAR0 (Base Address Register 0)
    uint32_t bar1 = pci_read(bus, device, function, 0x14); // Read BAR1 (Base Address Register 1)
    uint32_t bar2 = pci_read(bus, device, function, 0x18); // Read BAR2 (Base Address Register 2)
    uint32_t bar3 = pci_read(bus, device, function, 0x1C); // Read BAR3 (Base Address Register 3)
    uint32_t bar4 = pci_read(bus, device, function, 0x20); // Read BAR4 (Base Address Register 4)
    uint32_t bar5 = pci_read(bus, device, function, 0x24); // Read BAR5 (Base Address Register 5)
    // uint32_t bar6 = pci_read(bus, device, function, 0x28); // Read BAR6 (Base Address Register 6) // not present in all devices

    pci_device.base_address_registers[0] = bar0;           
    pci_device.base_address_registers[1] = bar1;
    pci_device.base_address_registers[2] = bar2;
    pci_device.base_address_registers[3] = bar3;
    pci_device.base_address_registers[4] = bar4;
    pci_device.base_address_registers[5] = bar5 & 0xFFFFFFF0;   // This is a pointer to the AHCI controller's memory address
    // pci_device.base_address_registers[6] = bar6;


    // printf(" [-] PCI: Device found at %d:%d.%d\n", bus, device, function);
    // printf("     Device ID: %x, Vendor ID: %x Class: %x, Subclass: %x, Prog IF: %x, Revision: %d\n",
    //     pci_device.device_id, pci_device.vendor_id, class, subclass, prog_if, pci_device.revision_id);

    print_device_info((pci_device_t *) &pci_device);

    return pci_device;
}



void print_device_info(pci_device_t *device){

    if(device == NULL){
        return;
    }

    switch(device->vendor_id){
        case VENDOR_INTEL:
            printf("     INTEL");
            break;

        case VENDOR_REALTEK:
            printf("     REALTEK");
            break;

        case VENDOR_BROADCOM:
            printf("     BROADCOM");
            break;

        case VENDOR_AMD:
            printf("     AMD");
            break;

        case VENDOR_NVIDIA:
            printf("     Vendor:VENDOR_NVIDIA");
            break;

        case VENDOR_VIA :
            printf("     VIA");
            break;

        case VENDOR_ATT:
            printf("     ATT");
            break;

        case VENDOR_QUALCOMM:
            printf("     QUALCOMM");
            break;

         case VENDOR_AQUANTIA:
            printf("     AQUANTIA");
            break;

        case VENDOR_MELLANOX:
            printf("     Vendor:VENDOR_MELLANOX");
            break;

        case VENDOR_VMWARE:
            printf("     VMWARE");
            break;

        case VENDOR_VIRTIO:
            printf("     VIRTIO");
            break;

        case VENDOR_QEMU:
            printf("     QEMU");
            break;
        
        default:
            printf("     Unknown");
            break;
    }

    printf(" Device ID: %d", device->device_id);

    switch(device->class_code){
        case PCI_CLASS_UNCLASSIFIED:
            printf("   PCI_CLASS_UNCLASSIFIED");
            switch(device->subclass_code){
                case PCI_SUBCLASS_NON_VGA:
                    printf("  NON_VGA\n");
                    break;
                case PCI_SUBCLASS_VGA_UNCLASSIFIED:
                    printf("  VGA_UNCLASSIFIED\n");
                    break;
            }
            break;

        case PCI_CLASS_SERIAL_BUS_CONTROLLER:
            printf("  SERIAL_BUS_CONTROLLER");
            switch(device->subclass_code){
                case PCI_SUBCLASS_FIREWIRE:
                    printf("  FIREWIRE\n");
                    break;
                case PCI_SUBCLASS_ACCESS:
                    printf("  ACCESS\n");
                    break;
                case PCI_SUBCLASS_SSA:
                    printf("  SSA\n");
                    break;
                case PCI_SUBCLASS_USB:
                    printf("  USB\n");
                    break;
                case PCI_SUBCLASS_FIBRE:
                    printf("  FIBRE\n");
                    break;
                case PCI_SUBCLASS_SMBUS:
                    printf("  SMBUS\n");
                    break;
                case PCI_SUBCLASS_INFINI_BAND:
                    printf("  INFINI_BAND\n");
                    break;
                case PCI_SUBCLASS_IPMI:
                    printf("  IPMI\n");
                    break;
                case PCI_SUBCLASS_SERCOS:
                    printf("  SERCOS\n");
                    break;
                case PCI_SUBCLASS_CANBUS:
                    printf("  FIREWIRE\n");
                    break;
                case 0x80:
                    printf("  OTHER\n");
                    break;
                default:
                    printf("  Unknown\n");
                    break;
            }
            break;

        case PCI_CLASS_BRIDGE:
            printf("      BRIDGE");
            switch(device->subclass_code){
                case PCI_SUBCLASS_HOST:
                    printf("  HOST\n");
                    break;
                case PCI_SUBCLASS_ISA:
                    printf("  ISA\n");
                    break;
                case PCI_SUBCLASS_EISA:
                    printf("  EISA\n");
                    break;
                case PCI_SUBCLASS_MCA:
                    printf("  MCA\n");
                    break;
                case PCI_SUBCLASS_PCI_TO_PCI_1:
                    printf("  PCI_TO_PCI_1\n");
                    break;
                case PCI_SUBCLASS_PCMCIA:
                    printf("  PCMCIA\n");
                    break;
                case PCI_SUBCLASS_NUBUS:
                    printf("  NUBUS\n");
                    break;
                case PCI_SUBCLASS_CARD_BUS:
                    printf("  CARD_BUS\n");
                    break;
                case PCI_SUBCLASS_RACE_WAY:
                    printf("  RACE_WAY\n");
                    break;
                case PCI_SUBCLASS_PCI_TO_PCI_2:
                    printf("  PCI_TO_PCI_2\n");
                    break;
                case PCI_SUBCLASS_INFINI_BAND_TO_PCI:
                    printf("  INFINI_BAND_TO_PCI\n");
                    break;
                default:
                    printf("  Unknown\n");
                    break;
            }
            break;

        case PCI_CLASS_MASS_STORAGE_CONTROLLER:
            printf("      MASS_STORAGE_CONTROLLER");
            switch(device->subclass_code){
                case PCI_SUBCLASS_IDE:
                    printf("  IDE\n");
                    break;
                case PCI_SUBCLASS_FLOPY_DISK:
                    printf(" FLOPY\n");
                    break;
                case PCI_SUBCLASS_IPI_BUS:
                    printf(" subclass IPI\n");
                    break;
                case PCI_SUBCLASS_RAID:
                    printf(" RAID\n");
                    break;
                case PCI_SUBCLASS_ATA:
                    printf(" ATA\n");
                    break;
                case PCI_SUBCLASS_SERIAL_ATA:
                    printf(" SATA\n");
                    break;
                case PCI_SUBCLASS_SCSI:
                    printf(" SCSI\n");
                    break;
                case PCI_SUBCLASS_NON_VOLATILE_MEM:
                    printf(" NVM\n");
                    break;
                default:
                    printf(" Unknown\n");
                    break;
            }
            break;

        case PCI_CLASS_NETWORK_CONTROLLER:
            printf("    NETWORK_CONTROLLER");
            switch(device->subclass_code){
                case PCI_SUBCLASS_ETHERNET:
                    printf(" ETHERNET\n");
                    break;
                case PCI_SUBCLASS_TOKEN_RING:
                    printf(" TOKEN RING\n");
                    break;
                case PCI_SUBCLASS_FDDI:
                    printf(" FDDI\n");
                    break;
                case PCI_SUBCLASS_ATM:
                    printf( " ATM\n");
                    break;
                case PCI_SUBCLASS_ISDN:
                    printf(" ISDN\n");
                    break;
                case PCI_SUBCLASS_WORLD_FIP:
                    printf(" WORLD FIP\n");
                    break;
                case PCI_SUBCLASS_PIC_MG:
                    printf(" PIC MG\n");
                    break;
                case PCI_SUBCLASS_INFINBAND:
                    printf( " INFINBAND\n");
                    break; 
                case PCI_SUBCLASS_FABRIC:
                    printf( " FABRIC\n");
                    break;
                default:
                    printf(" Unknown\n");
            }
            break;

        case PCI_CLASS_WIRELESS_CONTROLLER:
            printf("   WIRELESS_CONTROLLER");
            switch(device->subclass_code){
                case PCI_SUBCLASS_IRDA:
                    printf(" IRDA\n");
                    break;
                case PCI_SUBCLASS_IR:
                    printf(" IR\n");
                    break;
                case PCI_SUBCLASS_RF:
                    printf(" RF\n");
                    break;
                case PCI_SUBCLASS_BLUETOOTH:
                    printf(" BLUETOOTH\n");
                    break;
                case PCI_SUBCLASS_BROADBAND:
                    printf(" BROADBAND\n");
                    break;
                case PCI_SUBCLASS_ETHERNET_802_1_a:
                    printf(" ETHERNET_802_1_a\n");
                    break;
                case PCI_SUBCLASS_ETHERNET_802_1_b:
                    printf(" ETHERNET_802_1_b\n");
                    break;
                case 0x80:
                    printf(" Other\n");
                    break;
                default:
                    printf("  Unknown\n");
                    break;
            }
            break;

        case PCI_CLASS_PROCESSOR:
            printf("   PROCESSOR");
            break;

        case PCI_CLASS_DISPLAY_CONTROLLER:
            printf("     DISPLAY_CONTROLLER");
            switch(device->subclass_code){
                case PCI_SUBCLASS_VGA:
                    printf(" VGA\n");
                    break;
                case PCI_SUBCLASS_XGA:
                    printf(" XGA\n");
                    break;
                case PCI_SUBCLASS_3D:
                    printf(" 3D\n");
                    break;
                case 0x80:
                    printf(" Other\n");
                    break;
                default:
                    printf(" Unknown\n");
                    break;
            }
            break;

        case PCI_CLASS_INPUT_DEVICE_CONTROLLER:
            printf("  INPUT_DEVICE_CONTROLLER");
            switch(device->subclass_code){
                case PCI_SUBCLASS_KEYBOARD:
                    printf(" Keyboard\n");
                    break;
                case PCI_SUBCLASS_DIGITIZER_PEN:
                    printf(" Digitizer pen\n");
                    break;
                case PCI_SUBCLASS_MOUSE:
                    printf(" mouse\n");
                    break;
                case PCI_SUBCLASS_SCANNER:
                    printf(" scanner\n");
                    break;
                case PCI_SUBCLASS_GAMEPORT:
                    printf(" Gameport\n");
                    break;
                case 0x80:
                    printf(" Other\n");
                    break;
                default:
                    printf(" Unknown\n");
                    break;
            }
            break;

        default:
            printf("  Unknown Device\n");
            break;
    }
}


void pci_scan() {
    mass_storage_controllers = (pci_device_t *) kheap_alloc(sizeof(pci_device_t)*16, ALLOCATE_DATA);
    if(!mass_storage_controllers ){
        return;
    }

    network_controllers = (pci_device_t *) kheap_alloc(sizeof(pci_device_t)*16, ALLOCATE_DATA);
    if(!network_controllers ){
        return;
    }
    
    wireless_controllers = (pci_device_t *) kheap_alloc(sizeof(pci_device_t)*16, ALLOCATE_DATA);
    if(!wireless_controllers ){
        return;
    }

    printf("[PCI] Scanning PCI devices... \n");

    for (uint32_t index = 0; index < (256 * 32 * 8); index++) {
        uint8_t bus = (index >> 8) & 0xFF;
        uint8_t device = (index >> 3) & 0x1F;
        uint8_t function = index & 0x07;

        // read the device and  vendor id
        uint32_t _dev_vend = pci_read(bus, device, function, DEVICE_VENDOR_OFFSET);
        uint16_t device_id = _dev_vend >> 16;       // upper 16 bit
        uint16_t vendor_id = _dev_vend & 0xFFFF;    // lower 16 bi

        // Check if the device exists
        if (device_id == 0xFFFF || vendor_id == 0xFFFF) {
            continue; // Invalid device
        }
        
        pci_device_t found_device = pci_device_detect(bus, device, function);

        // Check if the device is a mass storage controller
        switch (found_device.class_code) {          // 0x1
            case PCI_CLASS_MASS_STORAGE_CONTROLLER:
                mass_storage_controllers[mass_storage_count++] = found_device; // Store the detected mass storage device
                 mass_storage_count++;
                break;

            case PCI_CLASS_NETWORK_CONTROLLER:  // 0x2
                network_controllers[network_controller_count++] = found_device; // Store the detected network controller
                network_controller_count++;
                break;
            
            case PCI_CLASS_DISPLAY_CONTROLLER:  // 0x3
                // printf(" [-] PCI: Display controller found at %d:%d.%d\n", bus, device, function);
                break;

            case PCI_CLASS_BRIDGE:  // 0x6
                // printf(" [-] PCI: Bridge found at %d:%d.%d\n", bus, device, function);
                break;

            case PCI_CLASS_SERIAL_BUS_CONTROLLER: // 0xC
                // printf(" [-] PCI: Serial bus controller found at %d:%d.%d\n", bus, device, function);
                break;

            case PCI_CLASS_WIRELESS_CONTROLLER: // 0xD
                wireless_controllers[wireless_controller_count++] = found_device; // Store the detected wireless controller
                wireless_controller_count++;
                break;

            default:
                break; // Not a mass storage or network controller
        }
    }
    printf("[PCI] PCI Scan Completed\n\n");
}



