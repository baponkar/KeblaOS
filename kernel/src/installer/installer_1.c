

#include "../../../ext_lib/limine-9.2.3/limine-bios-hdd.h"


#include "../driver/disk/disk.h"        // For writing MBR and GPT
#include "../fs/fatfs_wrapper.h"        // For fat32 functions
#include "../vfs/vfs.h"                 // For using fat32 and iso9660 functions
#include "../fs/FatFs-R.0.16/source/ff.h"

#include "../memory/kheap.h"

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"

#include "gpt.h"

#include "installer_1.h"



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


int esp_install(int iso_pd, int esp_pd){
    
    printf("\n Installing KeblaOS in Disk %d from Disk %d...\n", esp_pd, iso_pd);

    // ============================ ISO Disk Initialization ===========================

    // if(vfs_init(iso_pd) != 0){
    //     printf(" VFS Initialization in Disk %d is Failed!\n", iso_pd);
    //     return -1;
    // }
    // printf(" VFS Initialization in Bootable Disk %d is success!\n", iso_pd);
    
    // if(vfs_mount(iso_pd, 0, 0) != 0){
    //     printf(" VFS Mounted Bootable Disk %d is Failed!\n", iso_pd);
    //     return -1;
    // }
    // printf(" VFS Mount Bootable Disk %d is Success!\n", iso_pd);

    // ============================ ESP Disk Initialization ===========================
    if(vfs_init(esp_pd) != 0){
        printf(" VFS Initialization in Disk %d is Failed!\n", esp_pd);
        return -1;
    }
    printf(" VFS Initialization in Disk %d is success!\n", esp_pd);

    

    // if(vfs_mount(esp_pd, 0, 0) != 0){
    //     printf(" VFS Mounted Disk %d is Failed!\n", esp_pd);
    //     return -1;
    // }
    // printf(" VFS Mount Disk %d is Success!\n\n", esp_pd);

    // Mounting directly from fatfs layer to avoid issues
    char esp_path[4];
    snprintf(esp_path, sizeof(esp_path), "%d:", esp_pd);

    FATFS *fs_esp = (FATFS *)kheap_alloc(sizeof(FATFS), ALLOCATE_DATA);
    memset(fs_esp, 0, sizeof(FATFS));

    FRESULT res = f_mount(fs_esp, (const TCHAR*) esp_path, 0);
    if(res != FR_OK){
        printf("FATFS: Mounting failed on drive %d with error: %s\n", esp_pd, fatfs_error_string(res));
        return -1;
    }
    printf(" FATFS Mount Disk %d is Success!\n\n", esp_pd);


    // ============================= Creating Directories in ESP =======================
    for(int i = 0; boot_dirs[i] != NULL; i++){

        if(vfs_mkdir(esp_pd, boot_dirs[i]) != 0){
            printf(" VFS MKDIR in Path %s failed!\n", boot_dirs[i]);
            return -1;
        }
        printf(" VFS MKDIR in Path %s Success!\n", boot_dirs[i]);
    }
    printf(" Successfully created boot directories in Disk %d\n", esp_pd);

    return 0;
}





