
/*
    Kernel.c
    Build Date  : 16-12-2024
    Last Update : 15-01-2025
    Description : KeblaOS is a x86 architecture based 64 bit Operating System. Currently it is using Limine Bootloader.
    Reference   : https://wiki.osdev.org/Limine
                https://github.com/limine-bootloader/limine-c-template
                https://wiki.osdev.org/Limine_Bare_Bones
                https://wiki.osdev.org/SSE
                https://allthingsembedded.com/2018/12/29/adding-gpt-support-to-fatfs/
*/



#include "kmain.h"

bool debug_on = false;

extern uint64_t fb0_width;
extern uint64_t fb0_height;
extern uint64_t fb0_pitch;
extern uint16_t fb0_bpp;


extern ring_buffer_t* keyboard_buffer;          // To get the keyboard input

extern Disk *disks;
extern int disk_count; 

#define ESP_START_LBA 2048

int iso_disk_no = 0;    // The ISO SATAPI Disk which is mentioned in Makefile
int boot_disk_no = 1;   // The SATA Disk Which will be used to install KeblaOS

int boot_ld = 0;
int user_ld = 1;


void kmain(){

    serial_init("Successfully Serial Printing initialized!\n");

    get_bootloader_info();
    init_vga();
    
    // print_bootloader_info();

    get_set_memory();
        
    init_bs_cpu_core();

    // Initialize APIC and IOAPIC
    if(!has_apic()) printf("[Error] This System does not have APIC.\n");
        
    init_all_cpu_cores();    // Starts all CPU cores

    if(!pci_exists()){
        printf("[Error] This system do not have PCI!\n");
        return;
    }

    init_controllers();     // This have PCI Scan

    // Detect and Initialize all Disks
    if(kebla_get_disks() <= 0){
        printf("No Disk Found!\n");
        return;
    }
    printf("Total %d Disks Found.\n", disk_count);

    int progress;
    if(clear_disk(boot_disk_no, &progress) != 0){
        printf("Failed to clear Disk %d.\n", boot_disk_no);
    }else{
        printf("Successfully cleared Disk %d.\n", boot_disk_no);
    }
 
    // Sector layout for ESP and Data partitions
    uint64_t esp_start_lba = 2048;    // 2048
    uint64_t total_sectors = disks[boot_disk_no].total_sectors;
    uint64_t sector_size = disks[boot_disk_no].bytes_per_sector;
    uint64_t esp_sectors = 100 * 1024 * 1024 / sector_size;     // 100MB ESP
    uint64_t data_sectors = total_sectors - esp_sectors - 2048; // Rest for Data Partition

    // Create GPT enable ESP and Data Partitions
    if(create_esp_and_data_partitions(boot_disk_no, esp_sectors, data_sectors, total_sectors)){
        printf("Created ESP and Data Partitions on Disk %d successfully.\n", boot_disk_no);
    }else{
        printf("Failed to create ESP and Data Partitions on Disk %d.\n", boot_disk_no);
    }

    // Create FAT32 Volume  at LBA 2048
    if(create_fat32_volume(boot_disk_no, esp_start_lba, (uint32_t)esp_sectors)){
        printf("Created FAT32 Volume on ESP Partition successfully.\n");
    }else{
        printf("Failed to create FAT32 Volume on ESP Partition.\n");
    }

    if(fat32_mkdir_root(boot_disk_no, "BOOT")){
        printf("Successfully created BOOT directory\n");
    }else{
        printf("Failed to create BOOT Directtory\n");
    }

    if(fat32_mkdir_root(boot_disk_no, "EFI")){
        printf("Successfully created EFI directory\n");
    }else{
        printf("Failed to create EFI Directtory\n");
    }

    if(fat32_create_test_file(boot_disk_no)){
        printf("Successfully created TEST.TXT\n");
    }else{
        printf("Failed to create TEST.TXT\n");
    }


    // Install KeblaOS in ESP Partition
    // if(esp_install(iso_disk_no, boot_disk_no) != 0){
    //     printf("Failed to ESP Install KeblaOS in Disk %d failed!\n", boot_disk_no);
    // }else{
    //     printf("Successfully KeblaOS is Installed(ESP) in Disk %d:%d\n", boot_disk_no, boot_ld);
    // }
    

    // if(disks[iso_disk_no].type != DISK_TYPE_SATAPI ||  disks[iso_disk_no].type == DISK_TYPE_AHCI_SATA || disks[iso_disk_no].type == DISK_TYPE_NVME){

    //     boot_disk_no = iso_disk_no;

    //     init_user_space(boot_disk_no, boot_ld, user_ld);
        
    // }else{
    //     if(is_os_installed(boot_disk_no, boot_ld) == false){
    //         if(uefi_install(iso_disk_no, boot_disk_no) != 0){
    //             printf("Failed to UEFI Install KeblaOS in Disk %d failed!\n", boot_disk_no);
    //         }else{
    //             printf("Successfully KeblaOS is Installed(UEFI) in Disk %d:%d\n", boot_disk_no, boot_ld);
    //         }

    //         if(create_user_dirs(boot_disk_no) != 0){
    //             printf("Failed to Create User Directories and Files in %d:%d\n", boot_disk_no, user_ld);
    //         }else{
    //             printf("Successfully Created directories and files in Disk %d:%d.\n", boot_disk_no, user_ld);
    //         }
    //     }
    // }
    
    
    // fatfs_test_1(boot_disk_no);
    // vfs_test(boot_disk_no);

    // if(fatfs_mkfs(boot_disk_no, FM_FAT32 | FM_SFD) == 0){
    //     printf(" Successfully FAT32 FS created.\n");
    // }
    
    // fatfs_test(boot_disk_no);
    // fatfs_test_1(boot_disk_no);
    // fatfs_test_multi_partition(boot_disk_no);
    // print_disk_sector(boot_disk_no, 2048, 1);
    
    // kfs_test();

    // int_syscall_init();
    // int_syscall_test();

    // init_syscall(0);   // Initialize syscall for BSP (CPU 0)
    // test_syscall();
    
    // mouse_init();

    // lvgl_test();
    // ugui_test_1();
    // desktop_init();

    // test_e1000_driver();
    // start();
    // test_e1000();


    // switch_to_core(3);

    // init_user_mode();

    // Load and parse kernel modules by using limine bootloader
    // get_kernel_modules_info();
    // print_kernel_modules_info();
    // load_user_elf_and_jump();

    // acpi_poweroff();
    // acpi_reboot();

    // while(true){
    //     printf("Time: %d, Random Number: %d\n", get_apic_ticks(), rand());
    //     sleep_seconds(0, 10);
    // }

    // switch_to_core(2);
    

    halt_kernel();
}












