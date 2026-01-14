/*
    Description: This Function will Install KeblaOS in a Disk from Bootable Disk.
    Developing Date: 18-10-2025
    Last Update: 18-10-2025
    Developer: Bapon Kar

    Reference:
    1 https://wiki.osdev.org/GPT
    2 https://wiki.osdev.org/CRC32
    3 https://wiki.osdev.org/Partition_Table
    4 https://wiki.osdev.org/FAT
    5 https://www.elm-chan.org/fsw/ff/
    6 https://wiki.osdev.org/UEFI
    7 https://wiki.osdev.org/Bootable_CD
    8 https://wiki.osdev.org/Bootable_Disk
    9 https://wiki.osdev.org/El-Torito
    10  https://wiki.osdev.org/MBR_(x86)
*/



#include "../../../ext_lib/limine-9.2.3/limine-bios-hdd.h"


#include "../driver/disk/disk.h"        // For writing MBR and GPT
#include "../fs/fatfs_wrapper.h"        // For fat32 functions
#include "../vfs/vfs.h"                 // For using fat32 and iso9660 functions

#include "../memory/kheap.h"

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"
#include "gpt_header.h"

#include "installer.h"

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
    "/boot",
    "/boot/limine",
    "/efi",
    "/efi/boot",
    NULL
};

static const char *boot_files[] = {
    "/boot.cat",

    "/boot/limine/limine_bios_cd.bin",      
    "/boot/limine/limine_bios_hdd.sys",
    "/boot/limine/limine_uefi_cd.bin",   

    "/boot/boot_loader_wallpaper.bmp",
    "/boot/kernel.bin",
    "/boot/limine.conf",
    "/boot/user_main.elf",

    "/efi/boot/bootx64.efi",
    NULL
};



#if FF_MULTI_PARTITION
#define FIRST_PARTITON_SIZE 100         // 100 MB
#define SECOND_PARTITION_PERCENTAGE 90  // Remaining 90%
#define SECTOR_SIZE 512                 // 512 Bytes
extern PARTITION VolToPart[FF_VOLUMES];
#define MB2SECTOR(mb)((mb * 1024 * 1024) / SECTOR_SIZE)
#endif

// Write only the stage2 portion of binary_limine_hdd_bin_data into safe LBA (e.g. 34)
bool write_limine_stage2_embedding(int disk_no) {

    uint32_t total_bytes = (uint32_t)sizeof(binary_limine_hdd_bin_data);
    if (total_bytes <= 512) {
        printf(" ✗ Limine binary too small\n");
        return false;
    }

    uint32_t stage2_bytes = total_bytes - 512;
    uint32_t stage2_sectors = (stage2_bytes + 511) / 512;

    // Choose LBA 34 (first usable LBA in typical GPT layouts) to avoid GPT header/entries.
    uint64_t target_lba = 34;

    // NOTE: if your first partition starts before target_lba, pick a different LBA or create a BIOS partition.
    if (!kebla_disk_write(disk_no, target_lba, stage2_sectors, (void*)(binary_limine_hdd_bin_data + 512))) {
        printf(" ✗ Failed to write Limine Stage-2 at LBA %llu\n", (uint64_t)target_lba);
        return false;
    }

    printf(" ✓ Wrote Limine Stage-2 (%u bytes, %u sectors) at LBA %llu\n",
           stage2_bytes, stage2_sectors, (uint64_t)target_lba);

    return true;
}

bool write_fat32_fs_at_2048(int disk_no) {

    Disk disk = disks[disk_no];
    uint64_t total_sectors = disk.total_sectors;

    uint8_t sector[512];
    memset(sector, 0, sizeof(sector));

    // Jump instruction & OEM name
    sector[0] = 0xEB; sector[1] = 0x58; sector[2] = 0x90;
    memcpy(&sector[3], "MSDOS5.0", 8);

    // Bytes per sector = 512
    sector[11] = 0x00; 
    sector[12] = 0x02;
    // Sectors per cluster = 8 (4 KiB)
    sector[13] = 0x08;
    // Reserved sectors = 32
    sector[14] = 0x20; sector[15] = 0x00;
    // FATs = 2
    sector[16] = 0x02;
    // Root directory entries (0 for FAT32)
    sector[17] = 0x00; sector[18] = 0x00;
    // Total sectors (0 here → use 32-bit value later)
    sector[19] = sector[20] = 0x00;
    // Media descriptor
    sector[21] = 0xF8;
    // FAT size (32-bit field later)
    sector[22] = sector[23] = 0x00;
    // Sectors per track, number of heads, hidden sectors, etc. …

    uint32_t hidden = 2048;
    memcpy(&sector[28], &hidden, 4);

    uint32_t tot_sec_32 = (uint32_t)(total_sectors - 2048);
    memcpy(&sector[32], &tot_sec_32, 4);

    uint32_t fat_size = 8192; // Example: 8192 sectors per FAT
    memcpy(&sector[36], &fat_size, 4);

    memcpy(&sector[71], "KEBLAOS ", 8); // Volume label
    memcpy(&sector[82], "FAT32   ", 8);

    sector[510] = 0x55; sector[511] = 0xAA;

    // Write boot sector
    return kebla_disk_write(disk_no, 2048, 1, sector);
}


static int prepare_boot_disk(int pd){

    // ================= Make Partition ===============================
    int ld_1 = 0;   // First Logical drive
    int ld_2 = 1;   // Second Logical drive

     // Drive "0:" -> partition 1
    VolToPart[ld_1].pd = pd; 
    VolToPart[ld_1].pt = ld_1 + 1;     // Partition 1

    // Drive "1:" -> partition 2
    VolToPart[ld_2].pd = pd;  
    VolToPart[ld_2].pt = ld_2 + 1;     // Partition 2

    // 100 MB for first partiotion and remaining for second partition
    uint32_t plist[] = {100*1024*2, 100};    

    uint8_t *work = malloc(65536);
    if(!work){
        printf("Memory allocation for working buffer work is failed!\n");
        return -1;
    }

    if(vfs_fdisk(pd, plist, work) != 0){
        printf("VFS FDISK in Disk %d Failed!\n", pd);
        return -1;
    }
    printf("VFS FDISK in Disk %d Success!Boot Partition: %d MB & User Partiotion remaining.\n", pd, 100);

    if(!work){
        free(work);
    }

    // ==================== 1st Partition ====================================
    if(vfs_mkfs(pd, ld_1, VFS_FAT32) != 0){
        printf("VFS MKFS in Disk %d: partiton %d failed!\n", pd, 1);
        return -1;
    }
    printf("VFS MKFS in Disk %d: partition %d Success!\n", pd, 2);

    if(vfs_mount(pd, ld_1) != 0){
        printf("VFS Mount id Disk %d: partition %d failed!\n", pd, 1);
        return -1;
    }
    printf("VFS Mount id Disk %d: partition %d Success!\n", pd, 1);

    // ==================== 2nd Partition ======================================
    if(vfs_mkfs(pd, ld_2, VFS_FAT32) != 0){
        printf("VFS MKFS in Disk %d: partiton %d failed!\n", pd, 2);
        return -1;
    }
    printf("VFS MKFS in Disk %d: partition %d Success!\n", pd, 2);

    if(vfs_mount(pd, ld_2) != 0){
        printf("VFS Mount id Disk %d: partition %d failed!\n", pd, 2);
        return -1;
    }
    printf("VFS Mount id Disk %d: partition %d Success!\n", pd, 2);

    return 0;
}

// Creating boot directories in pd and partition ld
static int create_boot_dirs(int pd, int ld){

    char path[128];

    snprintf(path, sizeof(path), "%d:", ld);

    if(vfs_chdrive(pd, path) != 0){
        printf("VFS Change Drive into %d: is failed!\n", ld);
        return -1;
    }
    // printf("VFS Change Drive into %d: is Success!\n", ld);
    

    for(int i = 0; boot_dirs[i] != NULL; i++){
        memset(path, 0, sizeof(path));
        snprintf(path, sizeof(path), "%d:%s", ld, boot_dirs[i]);    // Path: "0:"

        if(vfs_mkdir(pd, path) != 0){
            printf("VFS MKDIR in Path %s failed!\n", path);
            return -1;
            break;
        }
        // printf("VFS MKDIR in Path %s Success!\n", path);
    }

    printf("Successfully created boot directories in Disk %d in Partition %d\n", pd, ld);

    return 0;
}

// This function copy boot files from iso_disk into boot_disk
static int copy_boot_files(int iso_pd, int boot_pd, int ld){

    char iso_path[128];
    char boot_path[128];

    for(int i=0; iso_files[i] != NULL; i++){
        // ---------------- Manage ISO Boot Files --------------------------------
        memset(iso_path, 0, sizeof(iso_path));
        snprintf(iso_path, sizeof(iso_path), "%d:%s", iso_pd, iso_files[i]);

        void *iso_file = vfs_open(iso_pd, (char *)iso_files[i], FA_READ);
        if(!iso_file){
            printf("Open file %s in Disk %d is failed!\n", iso_files[i], iso_pd);
            return -1;
        }
        // printf("Success in opening file %s\n", iso_path);

        if (vfs_stat(iso_pd, iso_file, NULL) == 0){
            printf("Empty File: %s\n", iso_path);
            vfs_close(iso_pd, iso_path);
            return -1;
        }
        // printf("Success in statistics file %s\n", iso_path);

        int file_size = vfs_get_fsize(iso_pd, iso_file);
        if (file_size <= 0) {
            printf("Unable to get size for: %s\n", iso_path);
            vfs_close(iso_pd, iso_file);
            return -1;
        }
        // printf("Success in get file size %s, size: %d\n", iso_path, file_size);

        void *file_data = malloc(file_size);
        if (!file_data) {
            printf("Memory allocation failed for: %s\n", iso_path);
            vfs_close(iso_pd, iso_file);
            return -1;
        }
        

        uint32_t bytes_read = vfs_read(iso_pd, iso_file, file_data, file_size);
        if(bytes_read <= 0){
            printf(" Failed to read file %s\n", iso_path);
            return -1;
        }
        // printf("Successfully read %d bytes from %s\n", bytes_read, iso_path);

        if(vfs_close(iso_pd, iso_file) != 0){
            printf("%s close failed!\n", iso_path);
            return -1;
        }
        // printf("Successfully closed file %s\n", iso_path);

        if (bytes_read != file_size) {
            printf("Read error on %s (%u/%u bytes)\n", "/BOOT.CAT", bytes_read, file_size);
            free(file_data);
            return -1;
        }
        // printf("Success in read file %s\n", iso_path);

        // ---------------------------- Manage Boot Files -------------------------
        memset(boot_path, 0, sizeof(boot_path));
        snprintf(boot_path, sizeof(boot_path), "%d:%s", ld, boot_files[i]);

        // Create destination file
        void *fat_file = vfs_open(boot_pd, boot_path, FA_CREATE_ALWAYS | FA_WRITE);
        if (!fat_file) {
            printf("Failed to create destination: %s\n", boot_path);
            free(file_data);
            return -1;
        }
        // printf(" Success open file %s\n", boot_path);

        // Write contents
        uint32_t bytes_written = vfs_write(boot_pd, fat_file, file_data, file_size);
        if(bytes_written <= 0){
            printf(" Failed to write file %s\n", boot_path);
            return -1;
        }
        // printf(" Successfully written %d bytes in file %s\n", bytes_written, boot_path);

        if(vfs_sync(boot_pd, fat_file) != 0){
            printf(" Failed to sync to file %s\n", boot_path);
            return -1;
        }
        // printf(" Successfully sync file %s\n", boot_path);

        if(vfs_close(boot_pd, fat_file) != 0){
            printf(" Failed to close file: %s !\n", boot_path);
            return -1;
        }
        // printf(" Successfully closed file %s\n", boot_path);


        if (bytes_written != file_size) {
            printf("Write error on %s (%u/%u bytes)\n", boot_path, bytes_written, file_size);
            return -1;
        } 

        printf(">>>> Successfully copied %s into %s\n", iso_path, boot_path);

        free(file_data);
    }

    return 0;
}

// This function would install OS in boot_pd from iso_pd
int uefi_install(int iso_pd, int boot_pd){

    printf("Installing KeblaOS in Disk %d from Disk %d\n", boot_pd, iso_pd);


    // ============================ ISO Disk Initialization ===========================
    if(vfs_init(iso_pd) != 0){
        printf("VFS Initialization in Disk %d is Failed!\n", iso_pd);
        return -1;
    }
    printf("VFS Initialization in Bootable Disk %d is success!\n", iso_pd);
    
    if(vfs_mount(iso_pd, 0) != 0){
        printf("VFS Mounted Bootable Disk %d is Failed!\n", iso_pd);
        return -1;
    }
    printf("VFS Mount Bootable Disk %d is Success!\n", iso_pd);

    // ============================= Boot Disk Initialization ==========================

    if(vfs_init(boot_pd) != 0){
        printf("VFS Initialization in Boot Disk %d is failed!\n", boot_pd);
        return -1;
    }
    printf("VFS Initialization in Disk %d is success\n", boot_pd);

    if(prepare_boot_disk(boot_pd) != 0){
        printf("Prearing Main Disk %d is failed\n", boot_pd);
        return -1;
    }
    printf("Successfully Create two partition in Main disk %d and mounted\n", boot_pd);

    
    if(create_boot_dirs(boot_pd, 0) != 0){
        printf("Creating Directories in Main Disk %d Partition %d failed!\n", boot_pd, 0);
    }
    printf("Successfully Created directories in Main Disk %d partition %d\n", boot_pd, 0);

    if(copy_boot_files(iso_pd, boot_pd, 0) != 0){
        printf("Failed to Copy files from Boot Disk %d into Main Disk %d partition %d\n", iso_pd, boot_pd, 0);
        return -1;
    }
    printf("Successfully Copyied files from boot disk %d into main disk %d partition %d\n", iso_pd, boot_pd, 0);

    // ======================== Put Marker in Boot Directory =====================

    char *marker_path = "0:/.os_installed";

    void *mark_file = vfs_open(boot_pd, marker_path, FA_CREATE_ALWAYS | FA_WRITE);
    if(!mark_file){
        printf("Failed to Open %s in Disk %d: %d\n", marker_path, boot_pd, 0);
        return -1;
    }
    printf("Successfully Opened %s\n", marker_path);

    char *installation_text = "Successfully KeblaOS is installed in this disk.";

    if(vfs_write(boot_pd, mark_file, installation_text, strlen(installation_text)) == -1){
        printf("Failed to write into test.txt\n");
        return -1;
    }
    printf("Successfully written in %s\n", marker_path);

    if(vfs_sync(boot_pd, mark_file) == -1){
        printf("Failed to Sync the file %s\n", marker_path);
        return -1;
    }
    printf("Successfully Sync file %s\n", marker_path);

    if(vfs_close(boot_pd, mark_file) == -1){
        printf("Failed to Close The File %s\n", marker_path);
    }
    printf("Successfully Closed File %s\n", marker_path);

    return 0;
}

// Checking either OS in installed in boot_pd at partition ld
bool is_os_installed(int boot_pd, int ld){

    printf("Checking Installation KeblaOS in Disk %d...\n", boot_pd);

    if(vfs_init(boot_pd) != 0){
        printf("VFS Initialization in Disk %d is Failed!\n", boot_pd);
        return false;
    }
    printf("VFS Initialization in Disk %d is Success!\n", boot_pd);

    if(vfs_mount(boot_pd, ld) != 0){
        printf("Mount Faied in Disk %d Partition %d\n", boot_pd, ld);
        return false;
    }
    printf("Successfully Mount Disk %d Partition %d\n", boot_pd, ld);

    char boot_root[6];
    snprintf(boot_root, sizeof(boot_root), "%d:", ld);

    if(vfs_chdrive(boot_pd, boot_root) != 0){
        printf("Change Drive in user disk %d at partition %d\n", boot_pd, ld);
        return false;
    }
    printf("Successfully Change drive in %d at partition %d\n", boot_pd, ld);

    char *marker_path = "0:/.os_installed";

    void *mark_file = vfs_open(boot_pd, marker_path, FA_READ);
    if(!mark_file){
        printf("Failed to Open %s in Disk %d: %d\n", marker_path, boot_pd, 0);
        return false;
    }
    printf("Successfully opened %s\n", marker_path);

    char *comp_text = "Successfully KeblaOS is installed in this disk.";

    char buff[56];

    if(vfs_read(boot_pd, mark_file, buff, sizeof(buff)) <= 0){
        printf("Failed to read %s\n", marker_path);
        return false;
    }
    printf("Successfully read file %s\n", marker_path);

    if(strncmp(buff, comp_text, strlen(comp_text)) != 0 ){
        printf("text .os_installed not matched\n");
        return false;
    }
    printf("text in .os_installed matched\n");

    // char user_root[6];
    // snprintf(user_root, sizeof(user_root), "%d:", ld);

    // if(vfs_chdrive(boot_pd, user_root) != 0){
    //     printf("Change Drive in user disk %d at partition %d\n", boot_pd, ld);
    //     return false;
    // }
    // printf("Successfully Change drive in %d at partition %d\n", boot_pd, ld);

    // char home_path[65];

    // snprintf(home_path, sizeof(home_path), "%d:/home/keblaos.txt", ld);

    // int res = vfs_stat(boot_pd, home_path, NULL);
    // if( res != 0){
    //     printf("%s not found! with error code: %d\n", home_path, res);
    //     vfs_unmount(boot_pd, ld);
    //     return false;
    // }
    // printf("%s found!\n", home_path);
    
    // for(int i = 0; boot_files[i] != NULL; i++){
    //     if(boot_files[i] == "/boot/kernel.bin" || boot_files[i] == "/boot.cat" || boot_files[i] == "/efi/boot/bootx64.efi"){
    //         continue;
    //     }
    //     memset(path, 0, sizeof(path));
    //     snprintf(path, sizeof(path), "%d:%s", ld, boot_files[i]);

    //     // if(!vfs_open(boot_disk_no, path, FA_READ)){
    //     //     printf("%s is not found in disk %d!\n", path, boot_disk_no);
    //     //     return false;
    //     // }
    //     // printf("%s is present in disk.\n", path);

    //     if(vfs_stat(boot_disk_no, path, NULL) != 0){
    //         printf("%s is not found in disk %d!\n", path, boot_disk_no);
    //         return false;
    //     }
    //     printf("%s is present in disk.\n", path);
    // }

    return true;
}

int create_user_dirs(int pd){

    int user_ld = 1;

    char root_path[4];
    snprintf(root_path, sizeof(root_path), "%d:", user_ld);
    
    if(vfs_chdrive(pd, root_path) != 0){
        printf("Changing Drive into Partition %d Failed!\n", user_ld);
        return -1;
    }
    printf("Successfully change drive %d\n", user_ld);

    // Creating Home Directory
    char home_path[64];
    snprintf(home_path, sizeof(home_path), "%d:/home", user_ld);

    if(vfs_mkdir(pd, home_path) != 0){
        printf("Failed to create directory %s\n", home_path);
        return -1;
    }
    printf("Successfully Created %s in Disk %d\n", home_path, pd);

    // Creating keblaos.txt file
    char file_path[64];
    snprintf(file_path, sizeof(file_path), "%d:/home/keblaos.txt", user_ld);

    // Creating a Test File
    void *test_file = vfs_open(pd, file_path, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if(!test_file){
        printf("Failed to open file %s\n", file_path);
        return -1;
    }
    printf("Successfully opened file %s\n", file_path);

    char data[64];
    memcpy(data, "KeblaOS\nThis file is created by Disk Initialization.\n", sizeof(data));

    if(vfs_write(pd, test_file, data, sizeof(data)) <= 0){
        printf("Failed to write data in %d:/home/keblaos.txt\n", user_ld);
        return -1;
    }
    printf("Successfully written data into %d:/home/keblaos.txt\n", user_ld);

    if(vfs_close(pd, test_file) != 0){
        printf("Failed to close the file %s\n", test_file);
        return -1;
    }
    printf("Successfully close the file %s\n", file_path);
}

void init_user_space(int boot_pd, int user_ld){

    if(vfs_init(boot_pd) != 0){
        printf("VFS Initialization in Disk %d is Failed!\n", boot_pd);
        return;
    }
    printf("VFS Initialization in Disk %d is Success!\n", boot_pd);

    char path[6];
    sprintf(path, "%d:/", user_ld);
    if(vfs_chdrive(boot_pd, path) != 0){
        printf("Change Drive %s failed!\n", path);
        return;
    }
    printf("Successfully Change drive %s\n", path);

    if(vfs_mount(boot_pd, user_ld) != 0){
        printf("VFS Mount id Disk %d: partition %d failed!\n", boot_pd, 2);
        return;
    }
    printf("VFS Mount id Disk %d: partition %d Success!\n", boot_pd, 2);
}