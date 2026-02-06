
/*
    This is the Disk wrapper on Low Level Drivers like AHCI, NVMe, etc.
    It provides a uniform interface for higher-level components to interact 
    with different types of disk drives without needing to understand the specifics of each driver.
*/

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/stdlib.h"

#include  "../../memory/vmm.h"
#include "../../memory/kheap.h"


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
        printf(" disks is NULL\n");
        return false;
    }
    if(disk_no > disk_count){
        printf(" disk_no: %d, disk_count: %d\n", disk_no, disk_count);
        return false;
    }

    Disk disk = disks[disk_no];
    
    if(disk.type == DISK_TYPE_UNKNOWN){
        printf(" Disk Type: %d\n", disk.type);
        return false;
    }

    if(disk.bytes_per_sector == 0){
        printf(" Byte per Sector: %d\n", disk.bytes_per_sector);
        return false;
    }

    if(disk.total_sectors == 0){
        printf(" Total Sectors: %d\n", disk.total_sectors);
        return false;
    }

    if(!disk.context){
        printf(" Context: %x\n", disk.context);
        return false;
    }

    return true;
}

// Initialize and detect disks connected to the system
int kebla_get_disks(){

    if(!disks){
        disks = (Disk *)kheap_alloc(sizeof(Disk) * MAX_TOTAL_DISKS, ALLOCATE_DATA);
        if(!disks) return -1;
    } 
    memset(disks, 0, sizeof(Disk) * MAX_TOTAL_DISKS);

    for(int i=0; i < MAX_TOTAL_DISKS; i++){
        disks[i].type = DISK_TYPE_UNKNOWN;
        disks[i].bytes_per_sector = 0;
        disks[i].total_sectors = 0;
        disks[i].context = NULL;
        disks[i].initialized = false;
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
                            // printf("controller no: %d, SATA Drive Found at port %d\n",c_idx, i);
                            disks[disk_count].type = DISK_TYPE_AHCI_SATA;
                            disks[disk_count].context = (void *)&abar->ports[i];    // Rebase the port
                            disks[disk_count].bytes_per_sector = sata_get_bytes_per_sector(&abar->ports[i]);
                            disks[disk_count].total_sectors = sata_get_total_sectors(&abar->ports[i]);
                            
                            if(disks[disk_count].bytes_per_sector <= 0 || disks[disk_count].total_sectors <= 0){
                                continue;
                            }else{
                                disks[disk_count].initialized = true;
                                if(debug_on) printf("[DISK] Disk No: %d, type:%d, Sector Space: %d Byte, Total Sectors: %d\n", 
                                disk_count, disks[disk_count].type, disks[disk_count].bytes_per_sector, disks[disk_count].total_sectors);
                            }

                            
                            disk_count++;
                        }else if(type == AHCI_DEV_SATAPI){
                            // printf("controller no: %d, SATAPI Drive Found at port %d\n", c_idx, i);
                            disks[disk_count].type = DISK_TYPE_SATAPI;
                            disks[disk_count].context = (void *)&abar->ports[i];	// Rebase the port
                            disks[disk_count].bytes_per_sector = satapi_get_bytes_per_sector(&abar->ports[i]);
                            disks[disk_count].total_sectors = satapi_get_total_sectors(&abar->ports[i]);
                            
                            if(debug_on) printf("[DISK] Disk No: %d, type:%d, Sector Space: %d Byte, Total Sectors: %d\n", 
                                disk_count, disks[disk_count].type, disks[disk_count].bytes_per_sector, disks[disk_count].total_sectors);
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
    

    if(disk_no < 0 || disk_no >= MAX_TOTAL_DISKS){
        if(debug_on) printf("[DISK] Invalid disk_no\n");
        return false;
    }

    if(!disks[disk_no].context) {
        if(debug_on) printf("[DISK] Disk %x context is NULL\n", disk_no);
        return false;
    }

    if(disks[disk_no].initialized){
        if(debug_on) printf("[DISK] Disk %d is already initialized\n", disk_no);
        return true;
    }

    if(disks[disk_no].type == DISK_TYPE_UNKNOWN){
        if(debug_on) printf("[DISK] Disk type is UNKNOWN!\n");
        return false;
    }else if(disks[disk_no].type == DISK_TYPE_AHCI_SATA){
        portRebase(disks[disk_no].context);
        Disk disk = disks[disk_no];

        uint64_t bytes_per_sector = sata_get_bytes_per_sector(disk.context);
        uint64_t total_sectors = sata_get_total_sectors(disk.context);

        if(bytes_per_sector <= 0 || total_sectors <= 0){
            return false;
        }

        disk.bytes_per_sector = bytes_per_sector;
        disk.total_sectors = total_sectors;
        disk.initialized = true;

        if(debug_on) printf("[DISK] Disk No: %d, Type: %d, Sector Size: %d Byte, Total Sectors: %d\n", 
            disk_no, disk.type, bytes_per_sector, total_sectors);

        if(debug_on) printf(" Successfully initialized AHCI SATA Disk %d\n", disk_no);
        
        
        return true;

    }else if(disks[disk_no].type == DISK_TYPE_NVME){

        if(debug_on) printf("[DISK] NVMe Filesystem Not implemented yet!\n");
        
        return false;

    }else if(disks[disk_no].type == DISK_TYPE_SATAPI){
        AtpiPortRebase(disks[disk_no].context);
        disks[disk_no].bytes_per_sector = satapi_get_bytes_per_sector(disks[disk_no].context);
        disks[disk_no].total_sectors = satapi_get_total_sectors(disks[disk_no].context);

        if(disks[disk_no].bytes_per_sector <= 0 || disks[disk_no].total_sectors <= 0){
            return false;
        }

        // Below values are set when iso9660 fs initialized
        disks[disk_no].root_directory_sector = 0;
        disks[disk_no].root_directory_size = 0;
        disks[disk_no].pvd_sector = 0;

        Disk disk = disks[disk_no];

        
        if(debug_on) printf("[DISK] Disk No: %d, type:%d, Sector Space: %d Byte, Total Sectors: %d\n", 
            disk_no, disk.type, disk.bytes_per_sector, disk.total_sectors);

        if(debug_on) printf(" Successfully initialized AHCI SATAPI Disk %d\n", disk_no);
        
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

    if(!buf){
        printf("[DISK] Buffer is NULL\n");
        return false;
    }

    if(disk.type == DISK_TYPE_AHCI_SATA){
        HBA_PORT_T *port = (HBA_PORT_T *) disk.context;
        if(!port){
            printf("[Disk] AHCI Port is NULL\n");
            return false;
        }
        return sata_read(port, lba, count, (uintptr_t)vir_to_phys((uint64_t)buf));
    }else if(disk.type == DISK_TYPE_NVME){
        printf("[DISK] NVMe Read Not implemented yet!\n");
        return false;
    }else if(disk.type == DISK_TYPE_SATAPI){
        HBA_PORT_T *port = (HBA_PORT_T *) disk.context;
        if(port == NULL){
            printf("[Disk] AHCI Port is NULL\n");
            return false;
        }
        return satapi_read(port, lba, count, buf);
    }else{
        printf("[DISK] Unsupported disk type %d\n", (uint64_t)disk.type);
        return false;
    }

    return false;   // Unsupported disk type
}

bool kebla_disk_write(int disk_no, uint64_t lba, uint32_t count, void* buf) {
    
    if(!disks){
        printf("[DISK] disks is NULL\n");
        return false;
    }

    if(disk_no > disk_count) return false;
    
    Disk disk = disks[disk_no];

    // ADD DEBUG INFO
    // printf("[DISK WRITE] Disk %d: lba=%x, count=%x, total=%x, buf=%x\n", 
    //        disk_no, lba, count, disk.total_sectors, buf);
    
    if(lba + count > disk.total_sectors) {
        printf("[DISK ERROR] Write exceeds boundary: %x (LBA) + %x (COUNT) = %x > %x (Total Sectors)\n",
               lba, count, lba + count, disk.total_sectors);
        return false;
    }

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
        void *ctx = disk.context;
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
        return false;
    }

    return false;   // Unsupported disk type
}

#define MAX_BATCH_SIZE 20480
int clear_disk(int disk_no, int *progress){

    printf("Formatting Disk %d: \n", disk_no);

    *progress = 0;

    if(!disks) return -1;

    if(disk_no < 0) {
        printf("Invalid disk number %d for formatting.\n", disk_no);
        return -1;
    }
    uint64_t total_sectors = disks[disk_no].total_sectors;
    const uint64_t SECTOR_SIZE = disks[disk_no].bytes_per_sector;



    uint8_t *buffer = kheap_alloc(SECTOR_SIZE * MAX_BATCH_SIZE, ALLOCATE_DATA);
    if (!buffer) {
        printf("Failed to allocate memory for formatting disk %d.\n", disk_no);
        return -1;
    }
    memset(buffer, 0, SECTOR_SIZE * 2048);

    for (uint64_t lba = 0; lba < total_sectors; lba += 2048) {
        *progress = (int)((lba * 100) / total_sectors);
            printf("\rProgress: %d %%", *progress);
        uint32_t sectors_to_write = (lba + 2048 <= total_sectors) ? 2048 : (total_sectors - lba);
        if (!kebla_disk_write(disk_no, lba, sectors_to_write, buffer)) {
            printf("Failed to format disk %d at LBA %d\n", disk_no, lba);
            kheap_free(buffer, SECTOR_SIZE * 2048);
            return -1;
        }
    }

    kheap_free(buffer, SECTOR_SIZE * 2048);

    printf("\n");

    return 0;
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


void kebla_disk_test(int disk_no){

    printf("[DISK] Testing Disk - %d....\n", disk_no);

    if(!kebla_disk_init(disk_no)){
        printf(" Disk initialization failed!\n");
    }
    printf(" Successfully Disk - %d (type: %d) initialized!\n", disk_no, DISK_TYPE_AHCI_SATA);

    if(!kebla_disk_status(disk_no)){
        printf(" Disk status check failed!\n");
        return;
    }

    Disk disk = disks[disk_no];
    printf(" Disk No: %d, Type: %d, byt. per sect.: %d, tot. sect.: %d, context: %x\n",
        disk_no, disks[disk_no].type, disks[disk_no].bytes_per_sector, 
        disks[disk_no].total_sectors, (uint64_t)disks[disk_no].context);

    const char* test_str = "KeblaOS Disk Test String!";
    
    switch(disk.type){
        case DISK_TYPE_AHCI_SATA:
            detect_partition_table(disk_no);

            uint8_t buffer[512];                        // Buffer to hold data (16 sectors of 512 bytes each)

            // Writing Test
            memcpy(buffer, test_str, strlen(test_str));
            if(!kebla_disk_write(disk_no, 2048, 1, (void *) buffer)){        // Writing At LBA 2048 in First Sector
                printf(" Write failed in disk - %d!\n", disk_no);
                return;
            }
            printf(" Write successful in disk %d.\n", disk_no);

            memset(buffer, 0, sizeof(buffer));          // Clearing the buffer

            // Reading Test
            memset(buffer, 0, sizeof(buffer));  // Clear buffer before reading
            if(!kebla_disk_read(disk_no, 2048, 1, buffer)){      // Reading LBA 0 and First sector into buffer
                buffer[511] = '\0';
                printf(" Read failed in disk - %d!.\n", disk_no);
                return;
            }
            printf(" Read successful, buffer content: %s\n", buffer);

            break;
        case DISK_TYPE_NVME:
            printf(" NVMe Disk Test Not implemented yet!\n");
            break;
        case DISK_TYPE_SATAPI:
            satapi_load((HBA_PORT_T *) disk.context);   // Load (Close Tray) before read/write

            // Writing Test not supported for SATAPI (CD/DVD)
            // memcpy(buffer, test_str, strlen(test_str));
            // if(!kebla_disk_write(disk_no, 2048, 1, (void *) str_data)){        // Writing At LBA 2048 in First Sector
            //     printf(" Write failed in disk - %d!\n", disk_no);
            //     return;
            // }

            // Reading Test
            uint8_t buff[2048];                             // Buffer to hold data (16 sectors of 512 bytes each)
            memset(buff, 0, sizeof(buff));                // Clear buffer before reading

            if(!kebla_disk_read(disk_no, 0, 1, buff)){      // Reading LBA 0 and First sector into buffer
                buffer[2047] = '\0';
                printf(" Read failed in disk - %d!.\n", disk_no);
                return;
            }
            printf(" Read successful, buffer content: %s\n", buff);
            break;
        default:
            printf(" Unsupported disk type: %d\n", (uint64_t)disk.type);
            return;
    }

    printf("[DISK] Test Disk - %d Completed!\n\n", disk_no);
}




void print_disk_sector(int disk_no, uint64_t lba, uint64_t count) {

    printf("Checking Disk %d Start Sector %d End Sector %d\n", disk_no, lba, lba + count - 1);
    char buff[512];

    for(int i = lba; i < lba+count; i++){
        if(!kebla_disk_read(disk_no, i, 1, buff)){
            printf(" Read Error Disk No: %d, LBA: %d", disk_no, i);
            return;
        }

        for(int j = 0; j < 512; j++){
            printf("%x ", buff[j]);
             if ((j + 1) % 16 == 0) printf("\n");
        }
        printf("\n\n");
        memset(buff, 0, sizeof(buff));
    }
}





