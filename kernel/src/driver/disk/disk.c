
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
#include "ahci/ahci.h"

#include "disk.h"

extern pci_device_t* mass_storage_controllers;     // Array to store detected mass storage devices
extern size_t mass_storage_count;           // Toal available Mass Storage Controllers (Defined in pci.c)


extern AHCI_Context *ahci_disks;            // Defined in AHCI
extern int ahci_disks_count;                // Defined in AHCI

Disk *disks;
int disk_count;


Disk *current_disk = NULL;


int get_total_disks(){
    return (int) disk_count;
}



bool kebla_disk_status(int disk_no){
    
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



bool kebla_disk_init(int disk_no){

    if(!disks){
        disks = (Disk *)kheap_alloc(sizeof(Disk) * mass_storage_count, ALLOCATE_DATA);
        if(!disks){
            printf("disk allocation failed!\n");
            return false;
        }
    }

    if(disk_no > mass_storage_count){
        printf("Invalid disk_no: %d\n", disk_no);
        return false;
    }

    pci_device_t device = mass_storage_controllers[disk_no];
    if(!device.base_address_registers){
        printf("mass_storage_controllers[disk_no] = NULL\n");
        return false;
    }

    if(device.class_code == PCI_CLASS_MASS_STORAGE_CONTROLLER){

        if(device.subclass_code == PCI_SUBCLASS_SERIAL_ATA){
            if(!init_ahci(disk_no)){
                printf("AHCI initialized Failed!\n");
                return false;
            }

            if(disk_no <= ahci_disks_count){
                AHCI_Context *ahci_disk = (AHCI_Context *)&ahci_disks[disk_no];
                if(!ahci_disk){
                    printf("ahci_disk is NULL\n");
                    return false;
                }

                Disk *disk = &disks[disk_no];
                disk->type = DISK_TYPE_AHCI;
                disk->bytes_per_sector = get_bytes_per_sector(ahci_disk->port);
                disk->total_sectors = get_total_sectors(ahci_disk->port);
                disk->context = (void *)ahci_disk;

                disk_count++;
            }else{
                printf("Invalid AHCI Disk No\n");
                return false;
            }
        }else if(device.subclass_code == PCI_SUBCLASS_NON_VOLATILE_MEM){
            disk_count++;
        }else if(device.subclass_code == PCI_SUBCLASS_IDE){
            disk_count++;
        }
    }

    return true;
}
    


bool kebla_disk_read(int disk_no, uint64_t lba, uint32_t count, void* buf){

    if(!disks){
        printf("disks is NULL\n");
        return false;
    }

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_NONE){
        printf("Disk %d is NULL\n", disk_no);
        return false;
    }

    if(disk.type == DISK_TYPE_AHCI){
        AHCI_Context *ctx =  (AHCI_Context *) disk.context;
        if(ctx == NULL)
        {
            printf("[Disk] AHCI Context is NULL\n");
            return false;
        }

        HBA_PORT_T *port = (HBA_PORT_T *) ctx->port;
        if(port == NULL){
            printf("[Disk] AHCI Port is NULL\n");
            return false;
        }
        return ahci_read(port, lba, count, (uintptr_t)vir_to_phys((uint64_t)buf));
    }

    return false;   // Unsupported disk type
}



bool kebla_disk_write(int disk_no, uint64_t lba, uint32_t count, void* buf) {
    
    if(!disks){
        printf("disks is NULL\n");
        return false;
    }
    
    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_NONE){
        printf("Disk %d is NULL\n", disk_no);
        return false;
    }

    if(disk.type == DISK_TYPE_AHCI){

        AHCI_Context *ctx =  (AHCI_Context *) disk.context;
        if(ctx == NULL)
        {
            printf("[Disk] AHCI Context is NULL\n");
            return false;
        }

        HBA_PORT_T *port = (HBA_PORT_T *) ctx->port;
        if(port == NULL){
            printf("[Disk] AHCI Port is NULL\n");
            return false;
        }
        return ahci_write(port, lba, count, (uintptr_t)vir_to_phys((uint64_t)buf));
    }
    return false;   // Unsupported disk type
}



int detect_partition_table(int disk_no) {
    uint8_t buffer[512];

    if (!kebla_disk_read(disk_no, 0, 1, buffer)) {
        printf(" Failed to read sector 0\n");
        return -1;
    }

    // Check signature
    if (buffer[510] != 0x55 || buffer[511] != 0xAA) {
        printf(" No valid MBR/GPT signature\n");
        return 0;
    }

    // Partition type at first entry
    uint8_t part_type = buffer[0x1BE + 4];
    if (part_type == 0xEE) {
        // Possible GPT → check LBA1
        if (!kebla_disk_read(disk_no, 1, 1, buffer)) {
            printf(" Failed to read GPT header\n");
            return -1;
        }
        if (memcmp(buffer, "EFI PART", 8) == 0) {
            printf(" Partition Table: GPT\n");
            return 2;
        }
    }

    printf(" Partition Table: MBR\n");
    return 1;
}


void disk_test(int disk_no){

    printf("[DISK] Testing AHCI Disk\n");

    if(!kebla_disk_init(disk_no)){
        printf(" Disk initialization failed!\n");
    }else{
        printf(" Successfully Disk initialized!\n");
    }

    for(int i=0; i<disk_count; i++){
        printf(" Disk No: %d, Type: %d, byt. per sect.: %d, tot. sect.: %d, context: %x\n",
            i, disks[i].type, disks[i].bytes_per_sector, 
            disks[i].total_sectors, (uint64_t)disks[i].context);
    }

    detect_partition_table(disk_no);

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_AHCI){

        uint8_t buffer[512];                        // Buffer to hold data (16 sectors of 512 bytes each)
        memset(buffer, 0, sizeof(buffer));          // Clearing the buffer
        
        char *str_data = "Hello from KeblaOS!";
        memcpy(buffer, str_data, strlen(str_data));

        if(kebla_disk_write(disk_no, 2048, 1, (void *) buffer)){        // Writing At LBA 2048 in First Sector
            printf(" Write successful from string into disk.\n");
        } else {
            printf(" Write failed from string into disk!\n");
            return;
        }

        // Clearing whole buffer before reading back
        memset(buffer, 0, sizeof(buffer));

        if(kebla_disk_read(disk_no, 2048, 1, buffer)){      // Reading LBA 0 and First sector into buffer
            buffer[511] = '\0';
            printf(" Read successful, buffer content: %s\n", buffer);

        } else {
            printf(" Read failed.\n");
        }
    } else {
        printf(" Unsupported disk type for testing.\n");
    }

    printf("[DISK] Test AHCI Disk Completed!\n");
}










