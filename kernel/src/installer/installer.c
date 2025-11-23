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

const char *iso_files[] = {
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

const char *fat32_dirs[] = {
    "/",
    "/boot",
    "/boot/limine",
    "/efi",
    "/efi/boot",
    NULL
};

const char *fat32_files[] = {
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


// This function will create directories in boot disk
void create_dirs(int disk_no){

    char fat_path[128];

    for(int i=0; fat32_dirs[i] != NULL; i++){
        memset(fat_path, 0, sizeof(fat_path));                                  // Clearing the memory
        snprintf(fat_path, sizeof(fat_path), "%d:%s", disk_no, fat32_dirs[i]);  // Create Dir Path and store into fat_path

        int res = vfs_mkdir(disk_no, fat_path);                                 // Creating directory in boot disk
        fat_path[127] = '\0';                                                   // Ensure null-termination
        if (res == 0) {
            printf(" ✓ Created: %s\n", fat_path);
        } else {
            printf(" ✗ Failed to create: %s\n", fat_path);
            continue;
        }
    }
}

// This function will copy files from iso dist into boot disk
void copy_fils(int boot_disk_no, int iso_disk_no) {
    printf("\n[INSTALLER] Copying system files from ISO to Boot Disk...\n");

    int files_copied = 0;
    int files_failed = 0;

    char iso_path[128];
    char fat_path[128];


    for (int i = 0; iso_files[i] != NULL && fat32_files[i] != NULL; i++) {
        memset(iso_path, 0, sizeof(iso_path));
        memset(fat_path, 0, sizeof(fat_path));                                 

        snprintf(iso_path, sizeof(iso_path), "%d:%s", iso_disk_no, iso_files[i]);       // Source path in ISO9660 
        snprintf(fat_path, sizeof(fat_path), "%d:%s", boot_disk_no, fat32_files[i]);    // Destination path in FAT32

        // Open source file on ISO
        void *iso_file = vfs_open(iso_disk_no, (char *) iso_files[i], FA_READ);
        if (!iso_file) {
            printf("   ✗ Source not found: %s\n",  iso_files[i]);
            files_failed++;
            continue;
        }

        // Check the presence of the file
        if (vfs_stat(iso_disk_no, iso_file, NULL) == 0) {
            printf("   ⚠ Empty file skipped: %s\n", iso_path);
            vfs_close(iso_disk_no, iso_file);
            continue;
        }

        // Get file size
        int file_size = vfs_get_fsize(iso_disk_no, iso_file);
        if (file_size <= 0) {
            printf("   ✗ Unable to get size for: %s\n", iso_path);
            vfs_close(iso_disk_no, iso_file);
            files_failed++;
            continue;
        }

        // Allocate temporary buffer
        void *file_data = kheap_alloc(file_size, ALLOCATE_DATA);
        if (!file_data) {
            printf("   ✗ Memory allocation failed for: %s\n", iso_path);
            vfs_close(iso_disk_no, iso_file);
            files_failed++;
            continue;
        }

        // Read ISO file contents
        uint32_t bytes_read = vfs_read(iso_disk_no, iso_file, file_data, file_size);
        vfs_close(iso_disk_no, iso_file);

        if (bytes_read != file_size) {
            printf("   ✗ Read error on %s (%u/%u bytes)\n", iso_path, bytes_read, file_size);
            kheap_free(file_data, file_size);
            files_failed++;
            continue;
        }

        // Create destination file
        void *fat_file = vfs_open(boot_disk_no, fat_path, FA_CREATE_ALWAYS | FA_WRITE);
        if (!fat_file) {
            printf("   ✗ Failed to create destination: %s\n", fat_path);
            kheap_free(file_data, file_size);
            files_failed++;
            continue;
        }

        // Write contents
        uint32_t bytes_written = vfs_write(boot_disk_no, fat_file, file_data, file_size);
        vfs_sync(boot_disk_no, fat_file);
        vfs_close(boot_disk_no, fat_file);
        // kheap_free(file_data, file_size);

        if (bytes_written != file_size) {
            printf("   ✗ Write error on %s (%u/%u bytes)\n", fat_path, bytes_written, file_size);
            files_failed++;
        } 
        printf("   ✓ Copied successfully (%u bytes) into %s\n", file_size, fat_path);
        files_copied++;
    }

    printf("\n[INSTALLER] Copy Summary:\n");
    printf("   ✓ Files Copied: %d\n", files_copied);
    printf("   ✗ Files Failed: %d\n", files_failed);
}


// Check either KeblaOS installed or not
bool is_keblaos_installed( int boot_disk_no) {

    uint8_t sector[512];

    printf("\nChecking if KeblaOS is installed on disk %d...\n", boot_disk_no);

    if(!kebla_disk_init(boot_disk_no)){
        printf(" Disk %d Initialization failed\n", boot_disk_no);
        return false;
    }
    printf(" Successfully Disk initialization in Disk %d\n", boot_disk_no);

    if(vfs_init(boot_disk_no) != 0 ){
        printf(" VFS Initialization failed in Disk %d\n", boot_disk_no);
        return false;
    }
    printf(" Successfully initialized VFS in Disk %d\n", boot_disk_no);

    
    if(vfs_disk_status(boot_disk_no) != 0){
        printf(" Disk %d Status error!\n", boot_disk_no);
        return false;
    }
    printf(" Disk %d Status ok\n", boot_disk_no);


    // ===== 1. Check Protective MBR =====
    if (!kebla_disk_read(boot_disk_no, 0, 1, sector)) {
        printf(" ✗ Failed to read MBR sector\n");
        return false;
    }

    // Signature 0x55AA at offset 510–511
    if (sector[510] != 0x55 || sector[511] != 0xAA) {
        printf(" ✗ Invalid MBR signature\n");
        return false;
    }

    // Partition type 0xEE for GPT protective MBR
    if (sector[450] != 0xEE) {
        printf(" ✗ Not a GPT Protective MBR (type=0x%x)\n", sector[450]);
        return false;
    }
    printf(" ✓ Protective MBR found\n");

    // ===== 2. Check GPT Header =====
    uint8_t gpt_header[512];
    if (!kebla_disk_read(boot_disk_no, 1, 1, gpt_header)) {
        printf(" ✗ Failed to read GPT header\n");
        return false;
    }

    // Check GPT signature "EFI PART"
    uint64_t sig = *(uint64_t*)gpt_header;
    if (sig != 0x5452415020494645ULL) {
        printf(" ✗ GPT header signature not found\n");
        return false;
    }
    printf(" ✓ GPT header found\n");

    // ===== 3. Check FAT32 filesystem presence =====
    FRESULT res = vfs_mount(boot_disk_no);
    if (res != FR_OK) {
        printf(" ✗ FAT32 mount failed: %s\n", fatfs_error_string(res));
        return false;
    }
    printf(" ✓ FAT32 filesystem mounted successfully\n");

    // ===== 4. Check for essential KeblaOS boot files =====
    for (int i = 0; fat32_files[i] != NULL; i++) {
        FILINFO fno;
        memset(&fno, 0, sizeof(FILINFO));

        char path[128];
        sprintf(path, "%d:%s", boot_disk_no, fat32_files[i]);

        res = vfs_stat(boot_disk_no, path, &fno);
        if (res != FR_OK) {
            printf(" ✗ Missing critical file: %s\n\n", path);
            if(vfs_unmount(boot_disk_no) != FR_OK){
                printf(" VFS Unmount failed in Disk %d\n", boot_disk_no);
            }
            return false;
        }
        printf(" ✓ Found: %s (%u bytes)\n", path, (unsigned)fno.fsize);
    }

    // ===== 5. Optional: verify kernel ELF signature =====
    void *kernel_buf = NULL;
    uint32_t kernel_size = 0;
    
    // Check kernel file format
    char kernel_path[128];
    sprintf(kernel_path, "%d:/boot/kernel.bin", boot_disk_no);
    if( vfs_open(boot_disk_no, kernel_path, FA_READ) == NULL) {
        printf(" ✗ Failed to open kernel file for verification\n\n");
        if(vfs_unmount(boot_disk_no) != FR_OK){
                printf(" VFS Unmount failed in Disk %d\n", boot_disk_no);
            }
        return false;
    }

    if(vfs_unmount(boot_disk_no) != FR_OK){
        printf(" VFS Unmount failed in Disk %d\n", boot_disk_no);
    }

    printf(" ✅ KeblaOS installation verified successfully on disk %d\n\n", boot_disk_no);

    return true;
}


void uefi_install(int boot_disk_no, int iso_disk_no){

    printf("\nInstalling KeblaOS in %d ...\n", boot_disk_no);

    if(!kebla_disk_init(iso_disk_no)){
        printf(" Disk %d Initialization failed\n", iso_disk_no);
        return;
    }else{
        printf(" Successfully Disk initialization in Disk %d\n", iso_disk_no);
    }
    
    // if(!kebla_disk_init(boot_disk_no)){
    //     printf(" Disk %d Initialization failed\n", boot_disk_no);
    //     return;
    // }else{
    //     printf(" Successfully Disk initialization in Disk %d\n", boot_disk_no);
    // }
    
    // ======================================= PMBR & GPT ==================================================================

    if(!create_complete_gpt(boot_disk_no)){
        printf(" Failed to write GPT Header!\n");
        return;
    }else{
        printf(" Successfully Complete GPT Header written!\n");
    }
    
    if(!write_limine_stage2_embedding(boot_disk_no)){
        printf(" Failed to Written bootloader!\n");
        return;
    }else{
        printf(" Successfully written Limine Stage 2\n");
    }
    
    // ================================= Making ISO9660 File System in ISO Disk ============================================
    if(vfs_init(iso_disk_no) != 0){
        printf(" Failed to initialize VFS in Disk %d\n", iso_disk_no);
        return;
    }else{
        printf(" Successfully initialized VFS in Disk %d\n", iso_disk_no);
    }

    // ================================= Making FAT32 Filesystem in Boot Disk ==============================================
    // if(vfs_init(boot_disk_no) != 0){
    //     printf(" Failed to initialize VFS in Disk %d\n", boot_disk_no);
    //     return;
    // }else{
    //     printf(" Successfully initialized VFS in Disk %d\n", boot_disk_no);
    // }

    if(vfs_mkfs(boot_disk_no, VFS_FAT32) != 0){
        printf(" VFS MKFS failed in Disk %d\n", boot_disk_no);
        return;
    }else{
        printf(" Successfully created FAT32 Filesystem in Disk %d at Sector 2048\n", boot_disk_no);
    }

    // ======================================= VFS Mounting ================================================================
    if(vfs_mount(iso_disk_no) != 0){
        printf(" VFS Mount failed in Disk %d\n", iso_disk_no);
        return;
    }else{
        printf(" Successfully mounted VFS in Disk %d\n", iso_disk_no);
    }
    
    if(vfs_mount(boot_disk_no) != 0){
        printf(" VFS Mount failed in Disk %d\n", boot_disk_no);
        return;
    }else{
        printf(" Successfully mounted VFS in Disk %d\n", boot_disk_no);
    }
    
    
    // ======================================= File Copying ================================================================
    create_dirs(boot_disk_no);
    copy_fils(boot_disk_no, iso_disk_no);

    printf(" Successfully Written bootloader\n");

    // ======================================== Success ======================================================================
    printf(" Installation complete!\n");
}




