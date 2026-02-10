/*
Version: 3.0
Details: This file will help to format disk with two GPT partition, Create FAT32 FS in First Partition(ESP),
         and  copy files from ISO disk into the disk's ESP so that it can UEFI BOOT.

*/

// Forr disks pointer
#include "../driver/disk/disk.h" 


// Library functions
#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"


// MY GPT Library
#include "gpt.h"

// MY FAT32 FS Library
#include "../fs/fat32/fat32.h"
#include "../fs/fat32/fat32_utility.h"

// MY ISO9660 FS Library
#include "../fs/iso9660/iso9660.h"

#include "installer_3.h"



#define ESP_START_LBA 2048
#define ESP_SIZE_IN_MB 100

extern const uint8_t binary_limine_hdd_bin_data[];

static const char *iso_files[] = {
    "/BOOT.CAT",

    "/BOOT/LIMINE/LIMINE_B.BIN",
    "/BOOT/LIMINE/LIMINE_B.SYS",
    "/BOOT/LIMINE/LIMINE_U.BIN",

    "/BOOT/BOOT_LOA.BMP",
    "/BOOT/KERNEL.BIN",
    "/BOOT/LIMINE.CON",
    "/BOOT/USER_MAI.ELF",

    "/EFI/BOOT/BOOTX64.EFI",
    NULL
};

static const char *boot_dirs[] = {
    "BOOT",
    "BOOT/LIMINE",
    "EFI",
    "EFI/BOOT",
    NULL
};


bool format_disk_and_install(int iso_disk_no, int boot_disk_no){

    if(iso_disk_no == boot_disk_no){
        printf("Iso Disk No %d, Boot Disk No %d\n", iso_disk_no, boot_disk_no);
        return false;
    }

    // Mounting ISO Disk to copy files from here
    if(iso9660_mount(iso_disk_no) != 0){
        printf("[INSTALLER] Failed to mount ISO9660 FS in Disk %d!\n", iso_disk_no);
    }else{
        printf("[INSTALLER] Successfully mount ISO9660 FS in Disk %d.\n", iso_disk_no);
    }

    // Clearing The Boot Disk
    int progress = 0;
    if(clear_disk(boot_disk_no, &progress) != 0){
        printf("[INSTALLER] Failed to erase data in Disk %d!\n", boot_disk_no);
        return false;
    }else{
        printf("[INSTALLER] Successfully erase all data in Disk %d.\n", boot_disk_no);
    }

    uint64_t esp_start_lba = ESP_START_LBA;                                 // 2048
    uint64_t total_sectors = disks[boot_disk_no].total_sectors;             // Total Sectors
    uint64_t sector_size = disks[boot_disk_no].bytes_per_sector;            // 512 Byte
    uint32_t esp_sectors = ESP_SIZE_IN_MB * 1024 * 1024 / sector_size;      // 100MB ESP
    uint64_t data_sectors = total_sectors - esp_sectors - esp_start_lba;    // Rest for Data Partition

    if(create_esp_and_data_partitions(boot_disk_no, esp_sectors, data_sectors, total_sectors)){
        printf("[INSTALLER] Created ESP and Data Partitions on Disk %d successfully.\n", boot_disk_no);
    }else{
        printf("[INSTALLER] Failed to create ESP and Data Partitions on Disk %d!\n", boot_disk_no);
        return false;
    }

    if(create_fat32_volume(boot_disk_no, esp_start_lba, esp_sectors)){
        printf("[INSTALLER] Created FAT32 Volume on ESP Partition successfully.\n");
    }else{
        printf("[INSTALLER] Failed to create FAT32 Volume on ESP Partition!\n");
        return false;
    }

    if(!fat32_mount(boot_disk_no, ESP_START_LBA)){
        printf("[INSTALLER] Failed to Mount FAT32 FS at LBA: %d!\n", ESP_START_LBA);
        return false;
    }else{
        printf("[INSTALLER] Successfully Mount Disk %d.\n", boot_disk_no);
    }

    for(int i=0; boot_dirs[i] != NULL; i++){
        if(fat32_mkdir(boot_disk_no, boot_dirs[i])){
            printf(" Successfully created %s Directory.\n", boot_dirs[i]);
        }else{
            printf(" Failed to create %s Directory!\n", boot_dirs[i]);
            return false;
        }
    }

    // ==================================================================================

    // Copy files in Boot Disk Boot Partition from iso disk
    for(int i=0; iso_files[i] != NULL; i++){

        char *path = iso_files[i];
        
        // printf(" Copying %s from ISO Disk %d...\n", path, iso_disk_no);

        iso9660_file_t st;

        if(iso9660_stat(iso_disk_no, path, &st) != 0){
            printf(" Empty File: %s\n", path);
            return false;
        }
        printf(" File %s present\n", path);

        void *iso_file = iso9660_open(iso_disk_no, path);
        if(!iso_file){
            printf(" Failed to open file %s in Disk %d\n", path, iso_disk_no);
            iso9660_close(iso_file);
            return false;
        } 

        uint32_t iso_file_size = iso9660_get_fsize(iso_file);
        if(iso_file_size <= 0){
            printf(" Unable to get size of %s\n", path);
            iso9660_close(iso_file);
            return false;
        }

        char *buf = (char *)malloc(iso_file_size);

        int iso_read_bytes = iso9660_read(iso_file, buf, iso_file_size);
        if(iso_read_bytes <= 0){
            printf(" Unable to read %s!\n", path);
            iso9660_close(iso_file);
            free(buf);
            return false;
        }
        printf(" Successfully read %d bytes from %s file\n", iso_file_size, path);

        // Find parent cluster
        uint32_t out_cluster;
        if(!fat32_path_to_cluster(iso_disk_no, path, &out_cluster)){
            printf("Failed to get parent cluster of %s\n");
            return false;
        }else{
            if(fat32_create_file_in_dir(iso_disk_no, out_cluster, path, buf)){
                printf("Successfully created %s\n", path);
            }
        }
    }

}

