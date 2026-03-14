
#include "../driver/disk/disk.h" 

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"

#include "../fs/fat32_fs/include/fat32.h"
#include "../fs/iso9660/iso9660.h"

#include "installer.h"


extern const uint8_t binary_limine_hdd_bin_data[];

#define SECTOR_SIZE 512


static const char *iso_files[] = {
    "/BOOT/KERNEL.BIN",
    "/BOOT/LIMINE.CON",
    "/BOOT/USER_MAI.ELF",

    "/EFI/BOOT/BOOTX64.EFI",
    NULL
};

static const char *files[] = {
    "/boot/kernel.bin",
    "/boot/limine.conf",
    "/boot/user_main.elf",

    "/efi/boot/bootx64.efi",
    NULL
};


static bool format_disk(int disk_no){
    
    // Clearing The Boot Disk
    size_t progress = 0;
    
    if(clear_disk(disk_no, &progress) != 0){
        printf("[INSTALLER] Failed to erase data in Disk %d!\n", disk_no);
        return false;
    }
    printf("[INSTALLER] Successfully erase all data in Disk %d.\n", disk_no);

    return true;
}


static bool create_partitions(int total_part, partition_t partition[]){

    if (!partition ) return false;

    for(int part_no = 0; part_no < total_part; part_no++){
        int pd_no = partition[part_no].pd_no;
        int ld_no = partition[part_no].ld_no;
        uint64_t start_lba = partition[part_no].start_lba;
        uint64_t sectors = partition[part_no].sectors;
        guid_t guid;
        memcpy(&guid, partition[part_no].guid, sizeof(guid_t));
        guid_t type_guid;
        memcpy(&type_guid, partition[part_no].type_guid, sizeof(guid_t));
        char *name = partition[part_no].name;

        if(!create_partition(pd_no, start_lba, sectors, guid, type_guid, name)){
            return false;
        }
    }

    return true;
}


bool uefi_install(int boot_disk_no, int main_disk_no, uint64_t esp_start_lba, uint64_t esp_sectors, uint64_t total_sectors){

    // Mounting ISO Disk to copy files from here
    if(iso9660_mount(boot_disk_no) != 0){
        printf("[INSTALLER] Failed to mount ISO9660 FS in Disk %d!\n", boot_disk_no);
        return false;
    }
    printf("[INSTALLER] Successfully mount ISO9660 FS in Disk %d.\n", boot_disk_no);

    // ----------------------------Ready BOOT Disk----------------------------------

    if(!format_disk(main_disk_no)){
        printf("[INSTALLER] Failed to format BOOT Disk\n");
        return false;
    }
    printf("[INSTALLER] Successfully format BOOT Disk\n");

    // ---------------- Creating ESP & DATA Partition ------------------------------

    fat32_set_disk(main_disk_no);

    int total_partitions = 2;

    partition_t partition[total_partitions];

    // Set value for ESP
    partition[0].pd_no = main_disk_no;
    partition[0].ld_no = 0;
    partition[0].start_lba = esp_start_lba;
    partition[0].sectors = esp_sectors;
    memcpy(partition[0].guid, ESP_GUID, sizeof(guid_t));
    memcpy(partition[0].type_guid, ESP_TYPE_GUID, sizeof(guid_t));
    char *esp_part_name = "ESP";

    // Set Value for DATA Partition
    partition[1].pd_no = main_disk_no;
    partition[1].ld_no = 1;
    partition[1].start_lba = esp_start_lba + esp_sectors;
    partition[1].sectors = total_sectors - (2 * esp_start_lba) - esp_sectors;   // 1MB for safety
    memcpy(partition[1].guid, DATA_PARTITION_GUID, sizeof(guid_t));
    memcpy(partition[1].type_guid, LINUX_FS_GUID, sizeof(guid_t));
    char *data_part_name = "DATA PARTITION";

    if(!create_partitions(total_partitions, partition)){
        printf("[INSTALLER] Failed to create Partitions\n");
        return false;
    }
    printf("[INSTALLER] Successfully Created %d partition\n", total_partitions);

    //---------------------------- ESP ---------------------------------------------------

    if (!create_fat32_volume( ESP_START_LBA, partition[0].sectors)) {
        printf("[INSTALLER] Failed to create FAT32 volume\n");
        return 1;
    }
    printf("[INSTALLER] Successfully Created FAT32 FS\n");

    if(!fat32_mount(esp_start_lba)){
        printf("[INSTALLER] Failed to Mount FAT32 FS at LBA: %d!\n", esp_start_lba);
        return false;
    }
    printf("[INSTALLER] Successfully Mount FAT32 FS at LBA %d\n", esp_start_lba);

    // Creating Directory inside BOOT Disk
    if(!f_mkdir("/boot")) return false;
    if(!f_mkdir("/efi")) return false;
    if(!f_mkdir("/efi/boot")) return false;

    // Copy Files from ISO Disk to BOOT Disk
    for(int i=0; iso_files[i] != NULL; i++){

        iso9660_file_t st;

        if(iso9660_stat(boot_disk_no, iso_files[i], &st) != 0){
            printf(" Empty File: %s\n", iso_files[i]);
            return false;
        }
        printf(" File %d:%s present\n", boot_disk_no, iso_files[i]);

        void *iso_file = iso9660_open(boot_disk_no,  iso_files[i]);
        if(!iso_file){
            printf(" Failed to open file %s in Disk %d\n", iso_files[i], boot_disk_no);
            iso9660_close(iso_file);
            return false;
        } 

        uint32_t fsize = iso9660_get_fsize(iso_file);
        if(fsize <= 0){
            printf(" Unable to get size of %s\n",  iso_files[i]);
            iso9660_close(iso_file);
            return false;
        }

        uint8_t *buf = (uint8_t *)malloc(fsize);
        if(!buf) return false;
        memset(buf, 0, fsize);

        int iso_read_bytes = iso9660_read(iso_file, buf, fsize);
        if(iso_read_bytes <= 0){
            printf(" Unable to read %s!\n",  iso_files[i]);
            iso9660_close(iso_file);
            free(buf);
            return false;
        }
        printf(" Successfully read %d bytes from %s file\n", fsize,  iso_files[i]);

  
        FAT32_FILE fp;
        int mode = FA_CREATE_ALWAYS | FA_WRITE;

        // Creating File
        if(!f_open(&fp, files[i], mode)){
            return false;
        }
        printf(" Successfully Created %s\n", files[i]);

        // Writing File
        uint32_t bw;
        if(!f_write(&fp, buf, fsize, &bw)){
            return false;
        }
        printf(" Successfully Write %s\n", files[i]);

        // Finally Close the file
        if(!f_close(&fp)){
            return false;
        }
        printf(" Successfully Close %s\n", files[i]);
        // ========================================================================================

        free(buf);
    }

    // ------------------------- DATA Partition---------------------------------------------------

    // if (!create_fat32_volume(partition[1].start_lba , partition[1].sectors)) {
    //     printf("[INSTALLER] Failed to create FAT32 volume\n");
    //     return 1;
    // }
    // printf("[INSTALLER] Successfully Created FAT32 FS\n");

    // if(!fat32_mount(partition[1].start_lba)){
    //     printf("[INSTALLER] Failed to Mount FAT32 FS at LBA: %d!\n", partition[1].start_lba);
    //     return false;
    // }
    // printf("[INSTALLER] Successfully Mount FAT32 FS at LBA %d\n", partition[1].start_lba);

    return true;
    
}


bool verify_installation(int disk_no, uint32_t start_lba)
{
    fat32_set_disk(disk_no);
    
    if(!fat32_mount( start_lba)){
        printf(" Failed to mount Disk %d at Sector %llu!\n", disk_no, start_lba);
        return false;
    }

    FAT32_STAT stat;

    // Check present of files
    if(!f_stat("/boot/kernel.bin", &stat)){
        printf(" /boot/kernel.bin not present\n");
        return false;
    } 

    if (!f_stat("/boot/limine.conf", &stat)){
        printf(" /boot/limine.conf not present\n");
        return false;
    } 

    if (!f_stat("/boot/user_main.elf", &stat)){
        printf(" /boot/user_main.elf not present\n");
        return false;
    }

    if (!f_stat("/boot/user_main.elf", &stat)){
        printf(" /boot/user_main.elf not present\n");
        return false;
    }

    if (!f_stat("/efi/boot/bootx64.efi", &stat)){
        printf(" /efi/boot/bootx64.efi file not present\n");
        return false;
    }

    return true;
}
