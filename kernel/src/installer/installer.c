/*
Version: 3.0
Details: The installer will do following:
    1. Format Disk
    2. Make GPT Disk with two partitions (ESP and DATA).
    3. Create FAT32 FS at Sector 2048 i.e. at ESP.
    4. Create necessary directories inside ESP's FAT32 FS.
    5. Copy files from ISO9660 Bootable Disk to the Given Boot Disk 
    
    The boot disk will boot KeblaOS successfully if the above steps are done correctly. 
    The installer will be tested on both real hardware and QEMU. The installer will be run in the kmain function, 
    and it will be controlled by a boolean variable named "install". When "install" is set to true, 
    the installer will run, otherwise it will not run. After the installation is done, 
    the "install" variable will be set to false, and the system will continue to boot normally.
Last Update: 14/09/2025
*/


// Forr disks pointer
#include "../driver/disk/disk.h" 


// Library functions
#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"


// MY GPT Library
#include "../fs/fat32_fs/include/gpt.h"
#include "../fs/fat32_fs/include/mbr.h"

// MY FAT32 FS Library
#include "../fs/fat32_fs/include/fat32.h"
#include "../fs/fat32_fs/include/fat32_utility.h"

// MY ISO9660 FS Library
#include "../fs/iso9660/iso9660.h"

// My VSFS FS Library
#include "../fs/vsfs/vsfs.h"

#include "../fs/ext2/ext2.h"

#include "../fs/fat32_fs/include/partition_manager.h"

#include "installer.h"



#define ESP_START_LBA 2048
#define ESP_SIZE_IN_MB 100

extern const uint8_t binary_limine_hdd_bin_data[];

static const char *iso_files[] = {
    // "/BOOT.CAT",

    // "/BOOT/LIMINE/LIMINE_B.BIN",
    // "/BOOT/LIMINE/LIMINE_B.SYS",
    // "/BOOT/LIMINE/LIMINE_U.BIN",

    // "/BOOT/BOOT_LOA.BMP",
    "/BOOT/KERNEL.BIN",
    "/BOOT/LIMINE.CON",
    "/BOOT/USER_MAI.ELF",

    "/EFI/BOOT/BOOTX64.EFI",
    NULL
};

static const char *parent_dir [] = {
    // "/",

    // "/boot/limine",
    // "/boot/limine",
    // "/boot/limine",

    // "/boot",
    "/boot",
    "/boot",
    "/boot",

    "/efi/boot",
    NULL
};

static const char *file_names[] = {
    // "BOOT.CAT",

    // "limine-bios-cd.bin",
    // "limine-bios.sys",
    // "limine-uefi-cd.bin",

//    "boot_loader_wallpaper.bmp",
    "kernel.bin",
    "limine.conf",
    "user_main.elf",

    "BOOTX64.EFI",
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

    
    uint64_t total_sectors = disks[boot_disk_no].total_sectors;             // Total Sectors
    uint64_t sector_size = disks[boot_disk_no].bytes_per_sector;            // 512 Byte
    
    uint64_t esp_start_lba = ESP_START_LBA;                                 // 2048
    uint32_t esp_sectors = ESP_SIZE_IN_MB * 1024 * 1024 / sector_size;      // 100MB ESP
    
    uint64_t data_start_lba =  esp_start_lba + esp_sectors;
    uint64_t data_sectors = total_sectors - esp_sectors - esp_start_lba - 2048;    // Rest for Data Partition, 2048 for safety

    // Creating ESP with FAT32 Filesystem
    if(create_partition(boot_disk_no, ESP_START_LBA, esp_sectors, ESP_GUID, ESP_TYPE_GUID, "ESP")){
        printf("Successfully created Partition at Sector %d\n", ESP_START_LBA);
    }

    if (!create_fat32_volume( ESP_START_LBA, esp_sectors)) {
        printf("Failed to create FAT32 volume\n");
        return 1;
    }

    // Creating Data Partition with FAT32 Filesystem
    if(create_partition(boot_disk_no, data_start_lba, data_sectors, DATA_PARTITION_GUID, LINUX_FS_GUID, "DATA PARTITION")){
        printf("Successfully created Partition at Sector %d\n", ESP_START_LBA);
    }

    if (!create_fat32_volume( data_start_lba, data_sectors)) {
        printf("Failed to create FAT32 volume\n");
        return 1;
    }

    // Mount Boot Partition with copy files from bootable disk
    if(!fat32_mount(ESP_START_LBA)){
        printf("[FAT32 TEST] Failed to Mount FAT32 FS at LBA: %d!\n", ESP_START_LBA);
        return false;
    }

    static const char *boot_dirs[] = {
        "boot",
        "boot/limine",
        "efi",
        "efi/boot",
        NULL
    };

    for(int i=0; boot_dirs[i] != NULL; i++){
        if(fat32_mkdir(boot_dirs[i])){
            printf(" Successfully created %s Directory.\n", boot_dirs[i]);
        }else{
            printf(" Failed to create %s Directory!\n", boot_dirs[i]);
            return false;
        }
    }

    // ==================================================================================
    
    if(fat32_change_current_directory( "/")){
        printf(" Successfully change current working directory /\n\n");
    }

    // Copy files in Boot Disk Boot Partition from iso disk
    for(int i=0; iso_files[i] != NULL; i++){

        printf(" Copying %s from ISO Disk %d:%s...\n", file_names[i], iso_disk_no, iso_files[i]);

        char *iso_file_path = iso_files[i];
        char *parent_dir_path = parent_dir[i];
        char *file_name = file_names[i];

        iso9660_file_t st;

        if(iso9660_stat(iso_disk_no, iso_file_path, &st) != 0){
            printf(" Empty File: %s\n", iso_file_path);
            return false;
        }
        printf(" File %d:%s present\n", iso_disk_no, iso_file_path);

        void *iso_file = iso9660_open(iso_disk_no, iso_file_path);
        if(!iso_file){
            printf(" Failed to open file %s in Disk %d\n", iso_file_path, iso_disk_no);
            iso9660_close(iso_file);
            return false;
        } 

        uint32_t iso_file_size = iso9660_get_fsize(iso_file);
        if(iso_file_size <= 0){
            printf(" Unable to get size of %s\n", iso_file_path);
            iso9660_close(iso_file);
            return false;
        }

        char *buf = (char *)malloc(iso_file_size);

        int iso_read_bytes = iso9660_read(iso_file, buf, iso_file_size);
        if(iso_read_bytes <= 0){
            printf(" Unable to read %s!\n", iso_file_path);
            iso9660_close(iso_file);
            free(buf);
            return false;
        }
        printf(" Successfully read %d bytes from %s file\n", iso_file_size, iso_file_path);

        uint32_t dir_cluster = 0;

        // Find parent directory cluster
        if(!fat32_path_to_cluster(parent_dir_path, &dir_cluster)){
            printf(" Failed to get parent directory cluster of %s\n", parent_dir_path);
            return false;
        }
        
        // Creating file inside boot disk
        if(!fat32_create_file_in_dir( dir_cluster, file_name, buf, iso_read_bytes)){
            return false;
        }
        printf(" Successfully created %s\n\n", file_name);

        if(memcmp(file_name, "limine.conf", 11) == 0){
            if(!fat32_path_to_cluster( "/efi/boot", &dir_cluster)){
                return false;
            }

            if(!fat32_create_file_in_dir( dir_cluster, file_name, buf, iso_read_bytes)){
                return false;
            }
            printf(" Successfully created %s in /efi/boot/", file_name);
        }

        free(buf);
    }



    return true;

}


bool verify_installation(int disk_no, uint32_t start_lba)
{
    uint32_t cluster;

    if(!fat32_mount( start_lba))
        return false;

    if (!fat32_path_to_cluster( "/boot/kernel.bin", &cluster))
        return false;

    if (!fat32_path_to_cluster( "/efi/boot/bootx64.efi", &cluster))
        return false;

    if (!fat32_path_to_cluster( "/efi/boot/limine.conf", &cluster))
        return false;

    return true;
}

