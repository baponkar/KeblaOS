
#include "../../../lib/stdio.h"
#include "../../../lib/string.h"

#include "../../../memory/vmm.h"
#include "../../../memory/kheap.h"

#include "sata_disk.h"

#define MAX_SATA_DISKS 10

extern pci_device_t *mass_storage_controllers;  // Array to store detected mass storage controllers
extern int mass_storage_controllers_count;   // Counter for mass storage controllers

HBA_PORT_T **sata_disks = NULL;                 // Store Different AHCI Disks contexts
int sata_disks_count = 0;                       // Total disks count
// Valid Disk no starts from 0 to sata_disks_count-1


bool init_sata(){
    sata_disks = (HBA_PORT_T **) kheap_alloc(sizeof(HBA_PORT_T *) * MAX_SATA_DISKS, ALLOCATE_DATA);
    if(!sata_disks){
        printf("[SATA] sata_disks allocation have failed!\n");
        return false;
    }
    memset(sata_disks, 0, sizeof(HBA_PORT_T *) * MAX_SATA_DISKS);
    sata_disks_count = 0;

    if(!mass_storage_controllers){
        printf("[SATA] mass_storage_controllers is NULL\n");
        return false;
    }

    if(mass_storage_controllers_count <= 0){
        printf("[SATA] mass_storage_controllers_count is %d\n", mass_storage_controllers_count);
        return false;
    }


    for(int controllers_no = 0; controllers_no < mass_storage_controllers_count; controllers_no++){
        pci_device_t device = mass_storage_controllers[controllers_no];

        uint64_t bar5 = (uint64_t) (device.base_address_registers[5] & ~0xF);    // BAR5 is a Physically memory mapped address
        if(bar5 == 0) continue;                             // Skip if BAR5 is not set

        HBA_MEM_T *abar = (HBA_MEM_T *) phys_to_vir(bar5);  // Map to virtual address
        if(!abar) continue;                                 // Skip if mapping fails

        // Find a port with a SATA drive
        uint32_t pi = abar->pi;
        for (size_t i = 0; i < 32; i++) 
        {
            if (pi & 1)
            {
                if(checkType(&abar->ports[i]) == AHCI_DEV_SATA){
                    portRebase(abar, i);	                // Rebase the port
                    HBA_PORT_T *port = (HBA_PORT_T *) &abar->ports[i];	// Select the port
                    sata_disks[sata_disks_count++] = port;   // Store the port context
                }
                continue;                                   // Skip Othertype
            }
            pi >>= 1;
        }
        
    }

    printf("[SATA] Sucessfully initialized SATA Disk \n");

    return true;
}



// LBA=start_l+((uint64_t)start_h<<32)
// start_l=LBA&0xFFFFFFFF
// start_h=(LBA>>32)&0xFFFFFFFF
// The below function is taking physical address of buffer 
bool sata_read(HBA_PORT_T* port, size_t _lba, size_t _count, uintptr_t _buf_phys_addr){
    uint64_t lba = (uint64_t) _lba;
    uint32_t count = (uint32_t) _count;
    uintptr_t buf_phys_addr = _buf_phys_addr;

    uint64_t total_sectors = sata_get_total_sectors(port);
    if(total_sectors > 0 && lba + count >= total_sectors){
        printf("[SATA] No Space in AHCI SATA Disk\n");
        return false;
    }
    
	uint32_t start_l = (uint32_t)(lba & 0xFFFFFFFF);
	uint32_t start_h = (uint32_t)((lba >> 32) & 0xFFFFFFFF);
    return runCommand(ATA_CMD_READ_DMA_EX, 0, port, start_l, start_h, count, _buf_phys_addr);
}



// LBA=start_l+((uint64_t)start_h<<32)
// start_l=LBA&0xFFFFFFFF
// start_h=(LBA>>32)&0xFFFFFFFF
// The below function is taking physical address of buffer 
bool sata_write(HBA_PORT_T* port, size_t _lba, size_t _count, uintptr_t _phys_buf_addr) {

    uint64_t lba = (uint64_t) _lba;
    uint32_t count = (uint32_t) _count;
    uintptr_t phys_buf_addr = (uintptr_t) _phys_buf_addr;

    uint64_t total_sectors = sata_get_total_sectors(port);
    if(total_sectors > 0 && lba + count >= total_sectors){
        printf("[SATA] No Space in AHCI SATA Disk\n");
        return false;
    }

	uint32_t start_l = (uint32_t)(lba & 0xFFFFFFFF);
	uint32_t start_h = (uint32_t)((lba >> 32) & 0xFFFFFFFF);
    return runCommand(ATA_CMD_WRITE_DMA_EX, 1, port, start_l, start_h, count, phys_buf_addr);
}




uint64_t sata_get_total_sectors(HBA_PORT_T* port) {
    void *identify_buf = (void *) kheap_alloc(512, ALLOCATE_DATA); 					// Allocate 512 bytes for IDENTIFY data
    if (identify_buf == NULL) {
        printf(" [SATA] Memory allocation for IDENTIFY buffer failed!\n");
        return 0;
    }
    memset(identify_buf, 0, 512); // Clear the buffer
    uintptr_t phys_identify_buf = (uintptr_t) vir_to_phys((uint64_t) identify_buf); // Convert to physical address

    // Send IDENTIFY command
    if(runCommand(ATA_CMD_IDENTIFY, 0, port, 0, 0, 1, phys_identify_buf)){

        uint16_t* id_buf_16 = (uint16_t*) identify_buf;

        // Check if LBA48 is supported
        bool lba48_supported = id_buf_16[83] & (1 << 10);

        if (lba48_supported) {
            // Word 100 = low word, Word 103 = high word
            uint64_t total_sectors =
                ((uint64_t)id_buf_16[103] << 48) |
                ((uint64_t)id_buf_16[102] << 32) |
                ((uint64_t)id_buf_16[101] << 16) |
                (uint64_t)id_buf_16[100];
            return total_sectors;
        } else {
            // Fallback to 28-bit
            uint32_t total_sectors =
                ((uint32_t)id_buf_16[61] << 16) |
                (uint32_t)id_buf_16[60];
            return total_sectors;
        }
    } else {
        printf(" [SATA] IDENTIFY command failed.\n");
        return 0;
    }

    kheap_free(identify_buf, 512); // Free the allocated buffer

    return 0;
}


uint32_t sata_get_bytes_per_sector(HBA_PORT_T* port) {
    void *identify_buf = (void *) kheap_alloc(512, ALLOCATE_DATA);
    if (identify_buf == NULL) {
        printf(" [SATA] Memory allocation for IDENTIFY buffer failed!\n");
        return 512; // default
    }
    memset(identify_buf, 0, 512);
    uintptr_t phys_identify_buf = (uintptr_t) vir_to_phys((uint64_t) identify_buf);

    // Send IDENTIFY command
    if (runCommand(ATA_CMD_IDENTIFY, 0, port, 0, 0, 1, phys_identify_buf)) {
        uint16_t* id_buf_16 = (uint16_t*) identify_buf;

        // Word 106, bit 14 = "words per logical sector valid"
        bool logical_sector_valid = id_buf_16[106] & (1 << 14);

        if (logical_sector_valid) {
            uint32_t bytes_per_sector =
                ((uint32_t)id_buf_16[118] << 16) |
                (uint32_t)id_buf_16[117];
            if (bytes_per_sector != 0) {
                kheap_free(identify_buf, 512);
                return bytes_per_sector;
            }
        }

        // Default sector size = 512 bytes
        kheap_free(identify_buf, 512);
        return 512;

    } else {
        printf(" [SATA] IDENTIFY command failed.\n");
        kheap_free(identify_buf, 512);
        return 512;
    }
}




void sata_disk_identify(HBA_PORT_T* port) {
    uint64_t sectors = sata_get_total_sectors(port);
    uint64_t size_mb = (sectors * 512) / (1024 * 1024);

    printf(" [SATA] Disk Total Sectors: %d\n", sectors);
    printf(" [SATA] Disk Size: %d MB\n", size_mb);

    uint16_t *buf = (uint16_t*)kheap_alloc(512, ALLOCATE_DATA);
    if (!buf) return;

    if (!runCommand(FIS_TYPE_REG_H2D, 0, port, 0, 0, 0, (uintptr_t)buf)) {
        printf("[SATA] IDENTIFY DEVICE failed\n");
        kheap_free(buf, sizeof(buf));
        return;
    }

    char serial[21] = {0};
    char firmware[9] = {0};
    char model[41] = {0};

    for (int i = 0; i < 10; i++) {
        serial[i*2]   = (buf[10+i] >> 8) & 0xFF;
        serial[i*2+1] = buf[10+i] & 0xFF;
    }
    serial[20] = '\0';

    for (int i = 0; i < 4; i++) {
        firmware[i*2]   = (buf[23+i] >> 8) & 0xFF;
        firmware[i*2+1] = buf[23+i] & 0xFF;
    }
    firmware[8] = '\0';

    for (int i = 0; i < 20; i++) {
        model[i*2]   = (buf[27+i] >> 8) & 0xFF;
        model[i*2+1] = buf[27+i] & 0xFF;
    }
    model[40] = '\0';

    printf(" [SATA] SATA Disk: Model: %s, SN: %s, FW: %s\n", model, serial, firmware);

    kheap_free(buf, sizeof(buf));
}





void test_sata(int sata_disk_no)
{
	printf("[SATA] Start Testing SATA DISK.................\n");

    if(!init_sata()) printf("SATA initialization is failed!\n");

    HBA_PORT_T *port = sata_disks[sata_disk_no];

	printf(" [-] AHCI Device Type: ");
	switch(checkType(port)){
		case AHCI_DEV_SATAPI:
			printf("AHCI_DEV_SATAPI\n");
			break;
		case AHCI_DEV_SEMB:
			printf("AHCI_DEV_SEMB\n");
			break;
		case AHCI_DEV_PM:
			printf("AHCI_DEV_PM\n");
			break;
		default:
			printf("AHCI_DEV_SATA\n");
			break;
	}

	// Get total sectors and size
	sata_disk_identify(port);

	// Create a buffer for the command list
    void *buf_1_vir = (void *)kheap_alloc(0x8000, ALLOCATE_DATA);       // 32 KB buffer (overkill but fine for test)
    if (buf_1_vir == NULL) {
        printf(" [SATA] Buffer_1 Memory allocation failed!\n");
        return;
    }
    memset(buf_1_vir, 0, 512); // Clear first sector (only 512 bytes needed)
	uintptr_t buf_1_phys = (uintptr_t)vir_to_phys((uint64_t)buf_1_vir); // Convert to physical address

    // Step 2: Write a string into the buffer
    const char* test_string = "Hello from KeblaOS!\0";
    memcpy((char*)buf_1_vir, test_string, strlen(test_string)); 	    // Copy into buffer


    // Step 3: Write the buffer to disk
    if (sata_write(port, 0, 4, buf_1_phys)) {
        printf(" [SATA] Write successful from buf_1 into disk.\n");
    } else {
        printf(" [SATA] Write failed from buf_1 into disk!\n");
        return;
    }


	void * buf_2_vir = (void *)kheap_alloc(0x8000, ALLOCATE_DATA);      // 32 KB buffer (overkill but fine for test)
	if (buf_2_vir == NULL) {
		printf(" [SATA] Buffer_2 Memory allocation failed!\n");
		return;
	}
	memset(buf_2_vir, 0, 512); // Clear first sector (only 512 bytes needed)
	// Convert virtual address to physical for AHCI read
	uintptr_t buf_2_phys = (uintptr_t)vir_to_phys((uint64_t)buf_2_vir); // Convert to physical address

    // Step 5: Read back from disk
    if (sata_read(port, 0, 4, buf_2_phys)) {
		// Null terminate the string to safely print it
		((char*)buf_2_vir)[strlen(test_string)] = '\0';
        printf(" [SATA] Data read from disk: %s.\n", (char*)buf_2_vir);
    } else {
        printf(" [SATA] Read failed from disk!\n");
    }

    kheap_free((void *)phys_to_vir((uint64_t) buf_1_vir), 0x8000); 	    // Free memory
	kheap_free((void *)phys_to_vir((uint64_t) buf_2_vir), 0x8000);	    // Free memory

	printf("[SATA] Test completed successfully...............\n");

	return;
}



