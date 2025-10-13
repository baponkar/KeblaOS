
/*
    This is the Disk wrapper on Low Level Drivers like AHCI, NVMe, etc.
    It provides a uniform interface for higher-level components to interact 
    with different types of disk drives without needing to understand the specifics of each driver.
*/

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/stdlib.h"

#include  "../../memory/vmm.h"

#include "../pci/pci.h"

#include "ahci/sata_disk.h"
#include  "ahci/satapi.h"

#include "disk.h"

extern bool debug_on;

extern pci_device_t* mass_storage_controllers;     // Array to store detected mass storage devices
extern int mass_storage_controllers_count;

#define MAX_TOTAL_DISKS 32

Disk *disks = NULL;
int disk_count = 0;
Disk current_disk;


bool kebla_disk_status(int disk_no){

    if(!disks){
        return false;
    }
    if(disk_no > disk_count){
        return false;
    }

    Disk disk = disks[disk_no];
    
    if(disk.type == DISK_TYPE_UNKNOWN){
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

int kebla_get_disks(){

    if(!disks){
        disks = (Disk *)malloc(sizeof(Disk) * MAX_TOTAL_DISKS);
        memset(disks, 0, sizeof(Disk) * MAX_TOTAL_DISKS);
    } 

    for(int c_idx=0; c_idx < mass_storage_controllers_count; c_idx++){
        pci_device_t dev = mass_storage_controllers[c_idx];

        if(dev.class_code == 0x01){         // Mass Storage Class
            if(dev.subclass_code == 0x06){  // SATA Controller
                // AHCI or IDE
                if(dev.prog_if == 0x01){   // AHCI 1.0 Controller

                    uint64_t bar5 = (uint64_t) (dev.base_address_registers[5] & ~0xF);
                    if(bar5 == 0) return -1;                            // Skip if BAR5 is not set

                    HBA_MEM_T *abar = (HBA_MEM_T *) phys_to_vir(bar5);  // Map to virtual address
                    if(!abar) return -1;                                // Skip if mapping fails

                    for (size_t i = 0; i < 32; i++) 
                    {
                        HBA_PORT_T *port = &abar->ports[i];
                        int type = checkType(port);
                        if(type == AHCI_DEV_SATA){
                            printf("controller no: %d, SATA Drive Found at port %d\n",c_idx, i);
                            disks[disk_count].type = DISK_TYPE_AHCI_SATA;
                            disks[disk_count].context = (void *)&abar->ports[i];    // Rebase the port
                            disk_count++;
                        }else if(type == AHCI_DEV_SATAPI){
                            printf("controller no: %d, SATAPI Drive Found at port %d\n", c_idx, i);
                            disks[disk_count].type = DISK_TYPE_SATAPI;
                            disks[disk_count].context = (void *)&abar->ports[i];	// Rebase the port
                            disk_count++;
                        }else if(type == AHCI_DEV_SEMB){
                            // printf("Port: %d, AHCI Device Type: %d\n", i,  AHCI_DEV_SEMB);
                        }else if(type == AHCI_DEV_PM){
                                // printf("Port: %d, AHCI Device Type: %d\n", i, AHCI_DEV_PM);
                        }else if(type == AHCI_DEV_NULL){

                        }else{
                                       
                        }
                    }
                }else if(dev.prog_if == 0x00){ // IDE Controller
                    
                }else{ // Unknown SATA
                    
                }
            }else if(dev.subclass_code == 0x05){ // NVMe Controller
                
            }else if(dev.subclass_code == 0x04){ // SCSI Controller
                
            }else if(dev.subclass_code == 0x0E){ // SATAPI Controller
                
            }else if(dev.subclass_code == 0x03){ // IDE Controller
                
            }else if(dev.subclass_code == 0x02){ // Floppy Controller
                
            }else if(dev.subclass_code == 0x01){ // RAID Controller
                
            }else{ // Unknown Mass Storage Controller
                
            }
        }
    }

    return disk_count;
}

bool kebla_disk_init(int disk_no){

    if(disk_count <= 0){
        if(kebla_get_disks() <= 0){
            printf("[DISK] No Disk Found!\n");
        }
    }
    
    if(disk_no < 0 || disk_no >= MAX_TOTAL_DISKS){
        if(debug_on) printf("[DISK] Invalid disk_no\n");
        return false;
    }

    if(!disks[disk_no].context) return false;

    if(disks[disk_no].type == DISK_TYPE_UNKNOWN){
        if(debug_on) printf("[DISK] Disk type is UNKNOWN!\n");
        return false;
    }else if(disks[disk_no].type == DISK_TYPE_AHCI_SATA){
        portRebase(disks[disk_no].context);
        disks[disk_no].bytes_per_sector = sata_get_bytes_per_sector(disks[disk_no].context);
        disks[disk_no].total_sectors = sata_get_total_sectors(disks[disk_no].context);
        if(debug_on) printf("[DISK] Successfully initialized AHCI SATA Disk %d\n", disk_no);
        return true;
    }else if(disks[disk_no].type == DISK_TYPE_NVME){
        if(debug_on) printf("[DISK] NVMe Filesystem Not implemented yet!\n");
        return false;
    }else if(disks[disk_no].type == DISK_TYPE_SATAPI){
        AtpiPortRebase(disks[disk_no].context);
        disks[disk_no].bytes_per_sector = satapi_get_bytes_per_sector(disks[disk_no].context);
        disks[disk_no].total_sectors = satapi_get_total_sectors(disks[disk_no].context);

        // Below values are set when iso9660 fs initialized
        disks[disk_no].root_directory_sector = 0;
        disks[disk_no].root_directory_size = 0;
        disks[disk_no].pvd_sector = 0;

        if(debug_on) printf("[DISK] Successfully initialized AHCI SATAPI Disk %d\n", disk_no);
        return true;
    }else{
        if(debug_on) printf("[DISK] Currenty not supporting %d type disk type!\n", (uint64_t)disks[disk_no].type);
        return false;
    }

    return false;
}

bool kebla_disk_read(int disk_no, uint64_t lba, uint32_t count, void* buf){

    if(!disks){
        printf("[DISK] disks is NULL\n");
        return false;
    }

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_UNKNOWN){
        printf("[DISK] Disk %d is NULL\n", disk_no);
        return false;
    }

    if(disk.type == DISK_TYPE_AHCI_SATA){
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
    }else if(disk.type == DISK_TYPE_NVME){

        // implementing nvme disk read
    }else if(disk.type == DISK_TYPE_SATAPI){
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
        return satapi_read(port, lba, count, buf);
    }else if(disk.type == DISK_TYPE_SCSI){

    }

    return false;   // Unsupported disk type
}

bool kebla_disk_write(int disk_no, uint64_t lba, uint32_t count, void* buf) {
    
    if(!disks){
        printf("[DISK] disks is NULL\n");
        return false;
    }
    
    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_UNKNOWN){
        printf("[DISK] Disk %d is NULL\n", disk_no);
        return false;
    }

    if(disk.type == DISK_TYPE_AHCI_SATA){

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
    }else if(disk.type == DISK_TYPE_NVME){
        // implementing nvme disk read
    }else if(disk.type == DISK_TYPE_SATAPI){
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
        return satapi_write(port, lba, count, buf);
    }else if(disk.type == DISK_TYPE_SCSI){

    }

    return false;   // Unsupported disk type
}

int find_disk_type(int disk_no) {
    if(disk_no > MAX_TOTAL_DISKS | disk_no > disk_count-1) return -1;
    if(!disks) return -1;

    return disks[disk_no].type;
}



int get_total_disks(){
    return (int) disk_count;
}

int detect_partition_table(int disk_no) {
    if(disk_no > MAX_TOTAL_DISKS | disk_no > disk_count-1) return -1;
    if(!disks) return -1;
    Disk disk = disks[disk_no];
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

    if(debug_on) printf("[DISK] Partition Table: MBR\n");

    return 1;
}


void disk_test(int disk_no){

    printf("[DISK] Testing Disk - %d\n", disk_no);

    if(!kebla_disk_init(disk_no)){
        printf(" Disk initialization failed!\n");
    }else{
        printf(" Successfully Disk - %d (type: %d) initialized!\n", disk_no, DISK_TYPE_AHCI_SATA);
    }

    printf(" Disk No: %d, Type: %d, byt. per sect.: %d, tot. sect.: %d, context: %x\n",
        disk_no, disks[disk_no].type, disks[disk_no].bytes_per_sector, 
        disks[disk_no].total_sectors, (uint64_t)disks[disk_no].context);

    detect_partition_table(disk_no);

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_AHCI_SATA){

        uint8_t buffer[512];                        // Buffer to hold data (16 sectors of 512 bytes each)
        memset(buffer, 0, sizeof(buffer));          // Clearing the buffer
        
        char *str_data = "Hello from KeblaOS!";
        memcpy(buffer, str_data, strlen(str_data));

        if(kebla_disk_write(disk_no, 2048, 1, (void *) str_data)){        // Writing At LBA 2048 in First Sector
            printf(" Write successful in disk %d.\n", disk_no);
        } else {
            printf(" Write failed in disk - %d!\n", disk_no);
            return;
        }


        if(kebla_disk_read(disk_no, 2048, 1, buffer)){      // Reading LBA 0 and First sector into buffer
            buffer[511] = '\0';
            printf(" Read successful, buffer content: %s\n", buffer);

        } else {
            printf(" Read failed.\n");
        }
    } else {
        printf(" Unsupported disk type for testing.\n");
    }

    printf("[DISK] Test Disk - %d Completed!\n\n", disk_no);
}










