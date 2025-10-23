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

#include "../fs/fatfs_wrapper.h"        // For fat32 functions

#include "../driver/disk/disk.h"            // For writing MBR and GPT

#include "../vfs/vfs.h"                     // For using fat32 and iso9660 functions

#include "../memory/kheap.h"

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"

#include "installer.h"


// CRC32 lookup table
static uint32_t crc32_table[256];
static int crc32_table_computed = 0;

// Generate CRC32 lookup table
static void generate_crc32_table(void) {
    if (crc32_table_computed) return;
    
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            if (c & 1) {
                c = 0xEDB88320 ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc32_table[i] = c;
    }
    crc32_table_computed = 1;
}

// Calculate CRC32 for data buffer
uint32_t crc32(const void *buf, size_t len){

    generate_crc32_table();
    
    const uint8_t *data = (const uint8_t *)buf;
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        uint32_t index = (crc ^ byte) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[index];
    }
    
    return ~crc;
}

// Function to create protective MBR at LBA 0
bool create_protective_mbr(int disk_no) {

    Disk disk = disks[disk_no];
    uint64_t total_sectors = disk.total_sectors;

    uint8_t mbr[512];
    memset(mbr, 0, sizeof(mbr));

    // ---- Partition Entry @ offset 0x1BE ----
    // Boot indicator
    mbr[446] = 0x00; // non-bootable

    // Starting CHS (C=0, H=0, S=2)
    mbr[447] = 0x00; // head
    mbr[448] = 0x02; // sector (2)
    mbr[449] = 0x00; // cylinder low

    // Partition type: 0xEE = GPT protective
    mbr[450] = 0xEE;

    // Ending CHS (max out)
    mbr[451] = 0xFF;
    mbr[452] = 0xFF;
    mbr[453] = 0xFF;

    // Starting LBA = 1
    mbr[454] = 0x01;
    mbr[455] = 0x00;
    mbr[456] = 0x00;
    mbr[457] = 0x00;

    // Size in sectors = total_sectors - 1 (limited to 32-bit)
    uint32_t size_lba = (total_sectors > 0xFFFFFFFFULL) ? 0xFFFFFFFF : (uint32_t)(total_sectors - 1);
    mbr[458] = (size_lba >> 0) & 0xFF;
    mbr[459] = (size_lba >> 8) & 0xFF;
    mbr[460] = (size_lba >> 16) & 0xFF;
    mbr[461] = (size_lba >> 24) & 0xFF;

    // ---- Boot signature ----
    mbr[510] = 0x55;
    mbr[511] = 0xAA;

    // ---- Write LBA 0 ----
    return kebla_disk_write(disk_no, 0, 1, mbr);
}



// Function to create GPT header at LBA 1
bool create_gpt_header(int disk_no){

    Disk disk = disks[disk_no];
    uint64_t total_sectors = disk.total_sectors;

    gpt_header_t header = {0};
    
    // GPT signature
    header.signature = 0x5452415020494645ULL;   // "EFI PART"
    header.revision = 0x00010000;
    header.header_size = sizeof(gpt_header_t);
    header.current_lba = 1;
    header.backup_lba = total_sectors - 1; 
    header.first_usable_lba = 34;
    header.last_usable_lba = total_sectors - 34; // Use actual disk size
    header.partition_entries_lba = 2;
    header.num_partition_entries = 128;
    header.partition_entry_size = sizeof(gpt_partition_entry_t);
    
    // Generate disk GUID
    for(int i = 0; i < 16; i++) {
        header.disk_guid[i] = 0x11 + i;
    }
    
    // Calculate header CRC32 (temporarily set to 0 for calculation)
    header.header_crc32 = 0;
    header.header_crc32 = crc32(&header, header.header_size);
    
    return kebla_disk_write(disk_no, 1, 1, &header);
}

// Function to create a complete GPT with one FAT32 partition at LBA 3
bool create_complete_gpt(int disk_no) {
    Disk disk = disks[disk_no];
    uint64_t total_sectors = disk.total_sectors;
    
    // Create partition entries array
    gpt_partition_entry_t partitions[128] = {0};
    
    // Create EFI system partition (first entry)
    uint8_t efi_type_guid[16] = {0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11, 
                                 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B};

    uint8_t fat32_type_guid[16] = {
        0xA2, 0xA0, 0xD0, 0xEB, // Data1
        0xE5, 0xB9,             // Data2
        0x33, 0x44,             // Data3
        0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 // Data4 (big-endian)
    };

    memcpy(partitions[0].partition_type_guid, fat32_type_guid, 16);
    
    // Generate partition GUID
    for(int i = 0; i < 16; i++) {
        partitions[0].unique_partition_guid[i] = 0x22 + i;
    }
    
    // FIX: Don't use entire disk - leave space for backup GPT
    partitions[0].starting_lba= 2048;
    partitions[0].ending_lba = total_sectors - 34;  // Leave last 33 sectors for backup GPT
    // Set EFI system partition attributes (bit 1 = legacy BIOS bootable)
    partitions[0].attributes = 0x8000000000000001; // OR 0x8000000000000001 for UEFI required
    
    // Partition name
    const char* name = "EFI System Partition";
    for(int i = 0; i < strlen(name) && i < 36; i++) {
        partitions[0].partition_name[i * 2] = name[i];  // UTF-16LE low byte
        partitions[0].partition_name[i * 2 + 1] = 0;    // UTF-16LE high byte
    }
    
    // Calculate partition entries CRC32
    uint32_t num_entries = 128;
    uint32_t entry_size = sizeof(gpt_partition_entry_t);
    uint32_t partition_entries_crc = crc32(partitions, num_entries * entry_size);
    
    // Create primary GPT header
    gpt_header_t primary_header = {0};
    primary_header.signature = 0x5452415020494645ULL;
    primary_header.revision = 0x00010000;
    primary_header.header_size = sizeof(gpt_header_t);
    primary_header.current_lba = 1;
    primary_header.backup_lba = total_sectors - 1;
    primary_header.first_usable_lba = 34;
    primary_header.last_usable_lba = total_sectors - 34;  // Last usable before backup area
    primary_header.partition_entries_lba = 2;
    primary_header.num_partition_entries = num_entries;
    primary_header.partition_entry_size = entry_size;
    primary_header.partition_entries_crc32 = partition_entries_crc;
    
    // Generate disk GUID
    for(int i = 0; i < 16; i++) {
        primary_header.disk_guid[i] = 0x11 + i;
    }
    
    // Calculate primary header CRC32
    primary_header.header_crc32 = 0;
    primary_header.header_crc32 = crc32(&primary_header, primary_header.header_size);
    
    // Create backup GPT header
    gpt_header_t backup_header = primary_header;
    backup_header.current_lba = total_sectors - 1;              // Backup header at end
    backup_header.backup_lba = 1;                               // Points to primary header
    backup_header.partition_entries_lba = total_sectors - 33;   // Backup entries before header
    
    // Calculate backup header CRC32
    backup_header.header_crc32 = 0;
    backup_header.header_crc32 = crc32(&backup_header, backup_header.header_size);
    
    // Write everything
    bool success = true;
    
    // Primary GPT structures
    success = success && kebla_disk_write(disk_no, 1, 1, &primary_header);      // Primary header
    success = success && kebla_disk_write(disk_no, 2, 32, partitions);          // Primary partition table (32 sectors)
    
    // Backup GPT structures  
    success = success && kebla_disk_write(disk_no, (total_sectors - 33), 32, partitions);  // Backup partition table
    success = success && kebla_disk_write(disk_no, (total_sectors - 1), 1, &backup_header); // Backup header
    
    return success;
}


void create_dirs(int disk_no){

    const char *fat32_dirs[] = {
        "/",
        "/boot",
        "/boot/limine",
        "/efi",
        "/efi/efi",
        NULL
    };

    char fat_path[128];

    for(int i=0; fat32_dirs[i] != NULL; i++){
        memset(fat_path, 0, sizeof(fat_path));                                  // Clearing the memory
        snprintf(fat_path, sizeof(fat_path), "%d:%s", disk_no, fat32_dirs[i]);  // Create Dir Path and store into fat_path

        int res = vfs_mkdir(disk_no, fat_path);
        fat_path[127] = '\0';                                                   // Ensure null-termination
        if (res == 0) {
            printf(" ✓ Created: %s\n", fat_path);
        } else {
            printf(" ✗ Failed to create: %s (%s)\n", fat_path, fatfs_error_string(res));
            return;
        }
    }
}


void copy_fils(int boot_disk_no, int iso_disk_no) {
    // ISO9660 → FAT32 mapping
    const char *iso_files[] = {
        "/BOOT.CAT",

        "/BOOT/LIMINE/LIMINE_B.BIN",
        "/BOOT/LIMINE/LIMINE_B.SYS",
        "/BOOT/LIMINE/LIMINE_U.BIN",

        "/BOOT/BOOT_LOA.BMP",
        "/BOOT/KERNEL.BIN",
        "/BOOT/LIMINE.CON",
        "/BOOT/LIMINE_0.BIN",
        "/BOOT/LIMINE_1.BIN",
        "/BOOT/LIMINE_B",
        "/BOOT/LIMINE_B.SYS",
        "/BOOT/LIMINE_U.BIN",
        "/BOOT/USER_MAI.ELF",

        "/EFI/BOOT/BOOTX64.EFI",
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
        "/boot/limine_0.bin",
        "/boot/limine_1.bin",
        "/boot/limine_bios_cd.bin",
        "/boot/limine_bios.sys",
        "/boot/limine_uefi_cd.bin",
        "/boot/user_main.elf",

        "/efi/boot/bootx64.efi",
        NULL
    };

    printf("\n[INSTALLER] Copying system files from ISO to Boot Disk...\n");

    int files_copied = 0;
    int files_failed = 0;
    char fat_path[128];

    for (int i = 0; iso_files[i] != NULL && fat32_files[i] != NULL; i++) {
        const char *iso_path = iso_files[i];            // Source path in ISO9660
        snprintf(fat_path, sizeof(fat_path), "%s", fat32_files[i]); // Destination path in FAT32


        // Open source file on ISO
        void *iso_file = vfs_open(iso_disk_no, (char *)iso_path, FA_READ);
        if (!iso_file) {
            printf("   ✗ Source not found: %s\n", iso_path);
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
        kheap_free(file_data, file_size);

        if (bytes_written != file_size) {
            printf("   ✗ Write error on %s (%u/%u bytes)\n", fat_path, bytes_written, file_size);
            files_failed++;
        } else {
            printf("   ✓ Copied successfully (%u bytes)\n", file_size);
            files_copied++;
        }
    }

    printf("\n[INSTALLER] Copy Summary:\n");
    printf("   ✓ Files Copied: %d\n", files_copied);
    printf("   ✗ Files Failed: %d\n", files_failed);
}



void uefi_install(int boot_disk_no, int iso_disk_no){

    printf("\nInstalling KeblaOS in %d ...\n", boot_disk_no);

    if(!vfs_init(iso_disk_no)){
        printf(" VFS Initialization failed in Disk %d\n", iso_disk_no);
        return;
    } 
    printf(" Successfully initialized VFS in Disk %d\n", iso_disk_no);
    

    if(!vfs_init(boot_disk_no)){
        printf(" VFS Initialization failed in Disk %d\n", boot_disk_no);
        return;
    }
    printf(" Successfully initialized VFS in Disk %d\n", boot_disk_no);
    

    if(!vfs_mkfs(boot_disk_no, VFS_FAT32)){
        printf(" VFS MKFS failed in Disk %d\n", boot_disk_no);
        return;
    }
    printf(" Successfully created FAT32 Filesystem in Disk %d\n", boot_disk_no);
    

    if(!vfs_mount(iso_disk_no)){
        printf(" VFS Mount failed in Disk %d\n", iso_disk_no);
        return;
    }
    printf(" Successfully mounted VFS in Disk %d\n", iso_disk_no);
    

    if(!vfs_mount(boot_disk_no)){
        printf(" VFS Mount failed in Disk %d\n", boot_disk_no);
        return;
    }
    printf(" Successfully mounted VFS in Disk %d\n", boot_disk_no);
    

    // Making Directories in Fat32 Disk and copy files from iso9660 fs 
    create_dirs(boot_disk_no);
    copy_fils(boot_disk_no, iso_disk_no);


    // ===================== PMBR & GPT ==========================================================================================
    
    // Updating MBR which already created by FatFS
    if (!create_protective_mbr(boot_disk_no)) {
        printf(" Failed to create Protective MBR!\n");
        return;
    } else {
        printf(" Successfully wrote Protective MBR at Sector 0\n");
    }
    
    // Updating GPT which previusly created by FatFS
    if (!create_complete_gpt(boot_disk_no)) {
        printf(" Failed to create complete GPT structure!\n");
        return;
    } else {
        printf(" Successfully wrote complete GPT structure\n");
    }

    printf(" Installation complete!\n");
}



bool is_keblaos_installed(int disk_no) {
    Disk disk = disks[disk_no];
    uint8_t sector[512];

    printf("\nChecking if KeblaOS is installed on disk %d...\n", disk_no);

    // ===== 1. Check Protective MBR =====
    if (!kebla_disk_read(disk_no, 0, 1, sector)) {
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
        printf(" ✗ Not a GPT Protective MBR (type=0x%02X)\n", sector[450]);
        return false;
    }

    printf(" ✓ Protective MBR found\n");

    // ===== 2. Check GPT Header =====
    uint8_t gpt_header[512];
    if (!kebla_disk_read(disk_no, 1, 1, gpt_header)) {
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
    FRESULT res = fatfs_mount(disk_no);
    if (res != FR_OK) {
        printf(" ✗ FAT32 mount failed: %s\n", fatfs_error_string(res));
        return false;
    }

    printf(" ✓ FAT32 filesystem mounted successfully\n");

    // ===== 4. Check for essential KeblaOS boot files =====
        const char *required_files[] = {
        "/boot.cat",
        "/boot/boot_loa.bmp",
        "/boot/kernel.bin",
        "/boot/limine/limine_b.bin",
        "/boot/limine/limine_b.sys",
        "/boot/limine/limine_u.bin",
        "/boot/limine.con",
        "/boot/limine_0.bin",
        "/boot/limine_1.bin",
        "/boot/user_mai.elf",
        
        "/boot/aa64.efi",
        "/efi/boot/bootia32.efi",
        "/boot/bootloon.efi",
        "/boot/bootrisc.efi",
        "/efi/boot/bootx64.efi",  
        NULL  // Terminator
    };

    for (int i = 0; required_files[i] != NULL; i++) {

        FILINFO fno;
        memset(&fno, 0, sizeof(FILINFO));

        res = fatfs_stat(required_files[i], &fno);
        if (res != FR_OK) {
            printf(" ✗ Missing critical file: %s\n", required_files[i]);
            fatfs_unmount(disk_no);
            return false;
        }
        printf(" ✓ Found: %s (%u bytes)\n", required_files[i], (unsigned)fno.fsize);
    }

    // ===== 5. Optional: verify kernel ELF signature =====
    void *kernel_buf = NULL;
    uint32_t kernel_size = 0;
    
    // Check kernel file format
    if( vfs_open(disk_no, "/boot/kernel.bin", FA_READ) == NULL) {
        printf(" ✗ Failed to open kernel file for verification\n");
        fatfs_unmount(disk_no);
        return false;
    }

    fatfs_unmount(disk_no);
    printf(" ✅ KeblaOS installation verified successfully on disk %d\n", disk_no);
    return true;
}

