/*
    Non-Volatile Memory Express (NVMe) Driver
    https://wiki.osdev.org/NVMe
    https://nvmexpress.org/specifications/
    https://alvinrolling.github.io/ssd/NVMe/
*/
#include "../../../lib/stdio.h"
#include "../../../lib/string.h"

#include "../../../memory/vmm.h"
#include "../../../memory/kheap.h"

#include "../../../driver/pci/pci.h"

#include "nvme.h"

extern bool debug_on;

extern pci_device_t* pci_devices;              // Array to store detected PCI devices
extern int pci_devices_count;                  // Counter for PCI devices

#define MAX_NVME_DISKS 10

uint64_t **nvme_disks = NULL;                   // An dynamic arrary Store Different NVMe disk's base address
int nvme_disks_count = 0;                       // Total disks count

uint64_t *nvme_base_addr = NULL;                // Current NVMe base address being used


uint32_t nvme_read_reg(uint32_t offset) {
	volatile uint32_t *nvme_reg = (volatile uint32_t *)(nvme_base_addr + offset);
	return *nvme_reg;
}

void nvme_write_reg(uint32_t offset, uint32_t value) {
	volatile uint32_t *nvme_reg = (volatile uint32_t *)(nvme_base_addr + offset);
	*nvme_reg = value;
}



bool init_nvme(){
    nvme_disks = (uint64_t **) kheap_alloc(sizeof(uint64_t *) * MAX_NVME_DISKS, ALLOCATE_DATA);
    if(!nvme_disks){
        printf("[NVME] sata_disks allocation have failed!\n");
        return false;
    }
    memset(nvme_disks, 0, sizeof(HBA_PORT_T *) * MAX_NVME_DISKS);
    nvme_disks_count = 0;


    for(int i = 0; i < pci_devices_count; i++){
        pci_device_t device = pci_devices[i];

        // Check if the device exists
        if (device.device_id == 0xFFFF || device.device_id == 0x0000 || device.vendor_id == 0xFFFF || device.vendor_id == 0x0000) {
            continue; // Invalid device
        }

        if( (device.class_code == 0x01) && (device.subclass_code == 0x08) && (device.prog_if == 0x02) ){
            if (debug_on) printf(" Found NVMe Device at %x:%x:%x\n", device.bus, device.device, device.function);
            if (debug_on) printf(" Device ID: %x, Vendor ID: %x, Class: %x, Subclass: %x\n", device.device_id, device.vendor_id, device.class_code, device.subclass_code);

            uint32_t bar0 = device.base_address_registers[0] & ~0xF; // Masking to get the address only
            uint32_t bar1 = device.base_address_registers[1] & ~0xF; // Masking to get the address only

            nvme_base_addr = (uint64_t *)(((uint64_t)bar1 << 32) | (bar0 & 0xFFFFFFF0));

            nvme_disks[nvme_disks_count++] = nvme_base_addr;

            printf(" NVMe MMIO Base Address: %x\n", (uint64_t) nvme_base_addr);

        }
    }

    printf("[NVME] Sucessfully initialized NVMe Disk \n");

    return true;
}




