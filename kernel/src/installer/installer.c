/*
Build Date: 13-10-2025
Written By: Bapon Kar
Downloaded from: https://github.com/baponkar/KeblaOS
Description: This file will install KeblaOS along with Limine bootloader into the selected disk 
             so that no more bootable disk or device need to start the KeblaOS.
*/


#include "../driver/disk/disk.h"

#include "../fs/FatFs-R0.15b/source/ff.h"
#include "../fs/fatfs_wrapper.h"
#include "../fs/iso9660.h"

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"

#include "installer.h"


// Limine bootstrap code for BIOS booting
static uint8_t limine_bootstrap_code[512] = {
    // Bootstrap code
    0xFA,                               // cli
    0x31, 0xC0,                         // xor ax, ax
    0x8E, 0xD8,                         // mov ds, ax
    0x8E, 0xC0,                         // mov es, ax
    0x8E, 0xD0,                         // mov ss, ax
    0xBC, 0x00, 0x7C,                   // mov sp, 0x7C00
    0xFB,                               // sti

    // Print "Limine..."
    0xB4, 0x0E,                         // mov ah, 0x0E
    0xB0, 'L', 0xCD, 0x10,              // mov al, 'L'; int 0x10
    0xB0, 'i', 0xCD, 0x10,              // mov al, 'i'; int 0x10
    0xB0, 'm', 0xCD, 0x10,              // mov al, 'm'; int 0x10
    0xB0, 'i', 0xCD, 0x10,              // mov al, 'i'; int 0x10
    0xB0, 'n', 0xCD, 0x10,              // mov al, 'n'; int 0x10
    0xB0, 'e', 0xCD, 0x10,              // mov al, 'e'; int 0x10
    0xB0, '.', 0xCD, 0x10,              // mov al, '.'; int 0x10
    0xB0, '.', 0xCD, 0x10,              // mov al, '.'; int 0x10
    0xB0, '.', 0xCD, 0x10,              // mov al, '.'; int 0x10

    // Prepare DAP (Disk Address Packet)
    0xBE, 0x50, 0x7C,                   // mov si, 0x7C50
    0x31, 0xC0,                         // xor ax, ax
    0x8E, 0xC0,                         // mov es, ax
    0x8A, 0x16, 0x80, 0x7C,             // mov dl, [0x7C80] (boot drive)
    0xB4, 0x42,                         // mov ah, 0x42
    0xCD, 0x13,                         // int 0x13
    0x73, 0x02,                         // jnc load_ok
    0xF4,                               // hlt (on error)
    // load_ok:
    0xEA, 0x00, 0x7E, 0x00, 0x00,       // jmp 0x0000:0x7E00

    // Disk Address Packet (DAP) at 0x7C50
    0x10, 0x00,                         // size=16, reserved=0
    0x01, 0x00,                         // read 1 sector
    0x00, 0x7E,                         // offset=0x7E00
    0x00, 0x00,                         // segment=0x0000
    0x00, 0x08, 0x00, 0x00,             // LBA low: 2048
    0x00, 0x00, 0x00, 0x00,             // LBA high: 0

    // Padding to make total 510 bytes of code + data
    // Fill remaining space with zeros
};




static bool write_limine_mbr(int disk_no) {
    uint8_t mbr_sector[512];
    memset(mbr_sector, 0, 512);
    
    // Copy Limine bootstrap code
    memcpy(mbr_sector, limine_bootstrap_code, sizeof(limine_bootstrap_code));
    
    // Set disk signature (optional)
    *(uint32_t*)&mbr_sector[440] = 0x12345678; // Example signature
    
    // Create a hybrid partition table
    MBRPartition *partitions = (MBRPartition*)&mbr_sector[446];
    
    // Partition 1: EFI System Partition (for UEFI boot)
    partitions[0].status = 0x00;        // Non-bootable for BIOS
    partitions[0].type = 0xEF;          // EFI System Partition
    partitions[0].lba_start = 1;        // Start at sector 1
    partitions[0].lba_size = 2048;      // 1MB EFI partition
    
    // Partition 2: Protective GPT partition (covers entire disk for GPT compatibility)
    partitions[1].status = 0x00;        // Non-bootable
    partitions[1].type = 0xEE;          // GPT protective partition
    partitions[1].lba_start = 1;        // Start after MBR
    partitions[1].lba_size = disks[disk_no].total_sectors - 1; // Rest of disk
    
    // Set boot signature
    mbr_sector[510] = 0x55;
    mbr_sector[511] = 0xAA;
    
    // Write MBR to disk
    return kebla_disk_write(disk_no, 0, 1, mbr_sector);
}


static void create_dirs(int disk_no){
    // Directories inside the ISO9660 filesystem
    const char *iso_dirs[] = {
        "/",            // root
        "/BOOT",
        "/BOOT/LIMINE", // Limine bootloader directory
        "/EFI",         // EFI directory
        "/EFI/BOOT",    // Boot directory for EFI
        NULL            // Terminator
    };

    for(int i=0; iso_dirs[i] != NULL; i++){
        char fat_path[256];
        snprintf(fat_path, sizeof(fat_path), "%d:%s", disk_no, iso_dirs);
        FRESULT res = fatfs_mkdir(iso_dirs[i]);
        if (res == FR_OK) {
            printf(" ✓ Created: %s\n", iso_dirs[i]);
        } else if (res == FR_EXIST) {
            printf("  Exists: %s\n", iso_dirs[i]);
        } else {
            printf(" ✗ Failed to create: %s (%s)\n", iso_dirs[i], fatfs_error_string(res));
        }
    }
}


static void copy_fils(int disk_no){

    // Files inside the ISO9660 filesystem (8.3 naming style)
    const char *iso_files[] = {
        // "/BOOT/BOOT_LOA.BMP",
        "/BOOT/KERNEL.BIN",
        "/BOOT/LIMINE/LIMINE_B.BIN",
        "/BOOT/LIMINE/LIMINE_B.SYS",
        "/BOOT/LIMINE/LIMINE_U.BIN",
        "/BOOT/LIMINE.CON",
        "/BOOT/USER_MAI.ELF",
        "/BOOT.CAT",
        "/EFI/BOOT/BOOTIA32.EFI",
        "/EFI/BOOT/BOOTX64.EFI",  
        NULL            // Terminator
    };

    // Copy Limine files
    printf("\n Copying Limine files...\n");

    int files_copied = 0;
    int files_failed = 0;

    for(int i=0; iso_files[i] != NULL; i++){
        const char* iso_path = iso_files[i];

        printf(" Checking: %s\n", iso_path);  // Debug output

        // Check if file exists on ISO
        void* file_data = NULL;
        uint32_t file_size = 0;

        if (iso9660_copy_file(2, iso_path, &file_data, &file_size)) {
            // printf("\n %s content: %s\n", iso_path, file_data);
            // File exists, create target path
            char fat_path[256];
            snprintf(fat_path, sizeof(fat_path), "%d:%s", disk_no, iso_path);
            
            // Open file on FAT32 for writing
            void* fat_file = fatfs_open(fat_path, FA_CREATE_ALWAYS | FA_WRITE);

            if (fat_file != NULL) {
                // Write file data using wrapper function
                int write_result = fatfs_write(fat_file, (char*)file_data, file_size);
                
                if (write_result == FR_OK) {
                    fatfs_sync(fat_file);
                    printf(" ✓ Copied: %s (%u bytes)\n", iso_path, file_size);
                    files_copied++;
                } else {
                    printf(" ✗ Write failed: %s (%s)\n", iso_path, fatfs_error_string(write_result));
                    files_failed++;
                }
                
                fatfs_close(fat_file);
            } else {
                printf(" ✗ Failed to create: %s\n", fat_path);
                files_failed++;
            }
            
            // Free the allocated file data
            free(file_data);
        }else{
            printf(" %s not exist in disk %d\n", iso_path, 2);
        }
    }

    printf("\n Copy Summary: Successfully copied %d files, Failed: %d files\n", files_copied, files_failed);
}




// Installing KeblaOS froom bootbale disk into selected disk
bool install_kebla_os(int boot_disk_no, int disk_no){

    printf("\n\n[Instal] Installing KeblaOS on Disk - %d...\n", disk_no);

    // ISO9660 Manage in Disk 1
    if (!iso9660_init(boot_disk_no)) {
        printf(" Failed to initialize ISO9660 filesystem\n");
        return false;
    }
    printf("[Install] Successfully ISO9660 filesystem initialized.\n");
    
    // FATFS Manage in Disk 0
    // Writing MBR at sector 0
    if(!write_limine_mbr(disk_no)){
        printf(" Failed to writing MBR in disk %d\n", disk_no);
        return false;
    }
    printf("[Install] Successfully Limine MBR is created into disk %d\n", disk_no);
  
    // Initializing Fatfs
    if(fatfs_init(disk_no) == FR_OK){
        printf("[Install] Successfully initialized FatFS in Disk %d\n", disk_no);
    }

    // Disk 0 Partition FAT32 at LBA Sector 2048 i.e. 1MB as diskio has LBA Offset 2048
    if(fatfs_mkfs(disk_no, FM_FAT32) == FR_OK){
        printf("[Install] Successfully created FAT32 Filesystem in Disk %d\n", disk_no);
    }

    // mount disk 0
    if(fatfs_mount(disk_no) == FR_OK){
        printf("[Install] Successfully Mount Disk %d\n", disk_no);
    }else{
        printf(" Disk %d mount failed!\n", disk_no);
    }

    // Creating Directories in Disk 0 like directories structures in Bootable Disk
    create_dirs(disk_no);

    // Copy Files from boot disk into Disk 0
    copy_fils(disk_no);

    return true;
}
























