
/*
This is the Disk wrapper on Low Level Drivers like AHCI, NVMe, etc.
It provides a uniform interface for higher-level components to interact 
with different types of disk drives without needing to understand the specifics of each driver.
*/

#include "../../lib/stdio.h"
#include "../../lib/string.h"

#include  "../../memory/vmm.h"
#include "../../memory/kheap.h"

#include "../pci/pci.h"
#include "ahci/sata_disk.h"

#include "disk.h"


extern pci_device_t* mass_storage_controllers;     // Array to store detected mass storage devices
extern int mass_storage_controllers_count;

#define MAX_DISKS 10

extern HBA_PORT_T **sata_disks; 
extern int sata_disks_count; 

Disk *disks = NULL;
int disk_count = 0;
Disk current_disk;


int get_total_disks(){
    return (int) disk_count;
}



bool kebla_disk_status(int disk_no){
    if(!disks){
        return false;
    }
    if(disk_no > disk_count){
        return false;
    }

    Disk disk = disks[disk_no];
    
    if(disk.type == DISK_TYPE_NONE){
        return false;
    }

    if(disk.bytes_per_sector == 0){
        return false;
    }

    if(disk.total_sectors == 0){
        return false;
    }

    if(!disk.context){
        return false;
    }

    return true;
}

void find_all_disks(){
    if(!mass_storage_controllers){
        printf("\n[DISK] No mass storage controllers found!\n");
        return;
    }
    if(mass_storage_controllers_count == 0){
        printf("\n[DISK] mass_storage_controllers_count is 0!\n");
        return;
    }
    
    printf("\n[DISK] Scanning for Mass Storage Controllers(total mass storage controllers %d)...\n", mass_storage_controllers_count);

    for(int i=0; i<mass_storage_controllers_count; i++){
        pci_device_t *dev = &mass_storage_controllers[i];
        // printf(" Mass Storage Controller %d: Vendor ID: %x, Device ID: %x, Class Code: %x, Subclass: %x, Prog IF: %x\n",
        //     i, dev->vendor_id, dev->device_id, dev->class_code, dev->subclass_code, dev->prog_if);

        if(dev->class_code == 0x01){   // Mass Storage Class
            if(dev->subclass_code == 0x06){ // SATA Controller
                // AHCI or IDE
                if(dev->prog_if == 0x01){   // AHCI 1.0
                    printf("  Found AHCI Controller: %x:%x\n", dev->vendor_id, dev->device_id);
                }else if(dev->prog_if == 0x00){ // IDE Controller
                    printf("  Found IDE Controller: %x:%x\n", dev->vendor_id, dev->device_id);
                }else{
                    printf("  Unknown SATA Prog IF: %x\n", dev->prog_if);
                }
            }else if(dev->subclass_code == 0x05){ // NVMe Controller
                printf("  Found NVMe Controller: %x:%x\n", dev->vendor_id, dev->device_id);
            }else if(dev->subclass_code == 0x04){ // SCSI Controller
                printf("  Found SCSI Controller: %x:%x\n", dev->vendor_id, dev->device_id);
            }else if(dev->subclass_code == 0x0E){ // SATAPI Controller
                printf("  Found SATAPI Controller: %x:%x\n", dev->vendor_id, dev->device_id);
            }else if(dev->subclass_code == 0x03){ // IDE Controller
                printf("  Found IDE Controller: %x:%x\n", dev->vendor_id, dev->device_id);
            }else if(dev->subclass_code == 0x02){ // Floppy Controller
                printf("  Found Floppy Controller: %x:%x\n", dev->vendor_id, dev->device_id);
            }else if(dev->subclass_code == 0x01){ // RAID Controller
                printf("  Found RAID Controller: %x:%x\n", dev->vendor_id, dev->device_id);
            }else{
                // printf("  Unknown Mass Storage Subclass: %x\n", dev->subclass_code);
            }
        }else{
            // printf("  Unknown Mass Storage Class: %x\n", dev->class_code);
        }
    }

    printf("[DISK] Scan for Mass Storage Controllers Completed!\n\n");
}


bool kebla_disk_init(int disk_no, DiskType type){

    // Creating disks array if not created yet
    if(!disks){
        disks = (Disk *)kheap_alloc(sizeof(Disk) * MAX_DISKS, ALLOCATE_DATA);
        if(!disks){
            printf("[DISK] disk allocation failed!\n");
            return false;
        }
        memset(disks, 0, sizeof(Disk) * MAX_DISKS);
    }

    printf("[DISK] mass_storage_controllers_count: %d\n", mass_storage_controllers_count);


    if(type == DISK_TYPE_NONE){
        printf("[DISK] Disk type is NONE!\n");
        return false;
    }else if(type == DISK_TYPE_SATA){
        if(!init_sata()){
            printf("[DISK] SATA initialization Failed!\n");
            return false;
        }

        // Set AHCI Disk
        HBA_PORT_T *sata_disk = sata_disks[disk_no];

        if(!sata_disk){
            printf("[DISK] No SATA disk found at index %d\n", disk_no);
            return false;
        }

        Disk disk = {0};
        disk.type = DISK_TYPE_SATA;
        disk.bytes_per_sector = sata_get_bytes_per_sector(sata_disk);
        disk.total_sectors = sata_get_total_sectors(sata_disk);
        disk.context = (void *)sata_disk;

        disks[disk_count++] = disk;
        
        current_disk = disk;
        printf("[DISK] SATA Disk %d initialized: bytes_per_sector=%d, total_sectors=%d\n", 
            disk_no, disk.bytes_per_sector, disk.total_sectors);
        return true;
    }else{
        printf("[DISK] Unsupported disk type!\n");
        return false;
    }

    return true;
}

    


bool kebla_disk_read(int disk_no, uint64_t lba, uint32_t count, void* buf){

    if(!disks){
        printf("[DISK] disks is NULL\n");
        return false;
    }

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_NONE){
        printf("[DISK] Disk %d is NULL\n", disk_no);
        return false;
    }

    if(disk.type == DISK_TYPE_SATA){
        void *ctx =  (void *) disk.context;
        if(ctx == NULL)
        {
            printf("[Disk] AHCI Context is NULL\n");
            return false;
        }

        HBA_PORT_T *port = (HBA_PORT_T *) ctx;
        if(port == NULL){
            printf("[Disk] AHCI Port is NULL\n");
            return false;
        }
        return sata_read(port, lba, count, (uintptr_t)vir_to_phys((uint64_t)buf));
    }

    return false;   // Unsupported disk type
}



bool kebla_disk_write(int disk_no, uint64_t lba, uint32_t count, void* buf) {
    
    if(!disks){
        printf("[DISK] disks is NULL\n");
        return false;
    }
    
    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_NONE){
        printf("[DISK] Disk %d is NULL\n", disk_no);
        return false;
    }

    if(disk.type == DISK_TYPE_SATA){

        void *ctx =   disk.context;
        if(ctx == NULL)
        {
            printf("[Disk] AHCI Context is NULL\n");
            return false;
        }

        HBA_PORT_T *port = (HBA_PORT_T *) ctx;
        if(port == NULL){
            printf("[Disk] AHCI Port is NULL\n");
            return false;
        }
        return sata_write(port, lba, count, (uintptr_t)vir_to_phys((uint64_t)buf));
    }
    return false;   // Unsupported disk type
}



int detect_partition_table(int disk_no) {
    uint8_t buffer[512];

    if (!kebla_disk_read(disk_no, 0, 1, buffer)) {
        printf("[DISK] Failed to read sector 0\n");
        return -1;
    }

    // Check signature
    if (buffer[510] != 0x55 || buffer[511] != 0xAA) {
        printf("[DISK] No valid MBR/GPT signature\n");
        return 0;
    }

    // Partition type at first entry
    uint8_t part_type = buffer[0x1BE + 4];
    if (part_type == 0xEE) {
        // Possible GPT → check LBA1
        if (!kebla_disk_read(disk_no, 1, 1, buffer)) {
            printf("[DISK] Failed to read GPT header\n");
            return -1;
        }
        if (memcmp(buffer, "EFI PART", 8) == 0) {
            printf("[DISK] Partition Table: GPT\n");
            return 2;
        }
    }

    printf("[DISK] Partition Table: MBR\n");
    return 1;
}


void disk_test(int disk_no){

    printf("[DISK] Testing AHCI Disk\n");

    if(!kebla_disk_init(disk_no, DISK_TYPE_SATA)){
        printf("[DISK] Disk initialization failed!\n");
    }else{
        printf("[DISK] Successfully Disk initialized!\n");
    }

    for(int i=0; i<disk_count; i++){
        printf("[DISK] Disk No: %d, Type: %d, byt. per sect.: %d, tot. sect.: %d, context: %x\n",
            i, disks[i].type, disks[i].bytes_per_sector, 
            disks[i].total_sectors, (uint64_t)disks[i].context);
    }

    detect_partition_table(disk_no);

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATA){

        uint8_t buffer[512];                        // Buffer to hold data (16 sectors of 512 bytes each)
        memset(buffer, 0, sizeof(buffer));          // Clearing the buffer
        
        char *str_data = "Hello from KeblaOS!";
        memcpy(buffer, str_data, strlen(str_data));

        if(kebla_disk_write(disk_no, 2048, 1, (void *) buffer)){        // Writing At LBA 2048 in First Sector
            printf("[DISK] Write successful from string into disk.\n");
        } else {
            printf("[DISK] Write failed from string into disk!\n");
            return;
        }

        // Clearing whole buffer before reading back
        memset(buffer, 0, sizeof(buffer));

        if(kebla_disk_read(disk_no, 2048, 1, buffer)){      // Reading LBA 0 and First sector into buffer
            buffer[511] = '\0';
            printf("[DISK] Read successful, buffer content: %s\n", buffer);

        } else {
            printf("[DISK] Read failed.\n");
        }
    } else {
        printf("[DISK] Unsupported disk type for testing.\n");
    }

    printf("[DISK] Test AHCI Disk Completed!\n");
}










