
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
#define ESP_SECTORS (100 * 1024 * 1024 / 512)   // 100 MB
#define DATA_PART_START_LBA (ESP_START_LBA + ESP_SECTORS)
#define DATA_PART_SECTORS ((1 * 1024 * 1024 * 1024 / 512) - ESP_SECTORS - ESP_START_LBA)

int iso_disk_no = 0;    // The ISO SATAPI Disk which is mentioned in Makefile
int boot_disk_no = 1;   // The SATA Disk Which will be used to install KeblaOS

bool install = false;  // This variable is set to true when the installer is running, otherwise it is false. It is used to control the flow of the installation process in kmain.

extern int disk_no;

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

    init_controllers();     // This function has PCI Scan

    // Detect and Initialize all Disks
    if(kebla_get_disks() <= 0){
        printf("No Disk Found!\n");
        return;
    }
    printf("[KMAIN] Total %d Disks Found.\n", disk_count);

    // for(int i=0; i < disk_count; i++){
    //     Disk disk = disks[i];

    //     if(disk.type == DISK_TYPE_SATAPI){
    //         // Mounting ISO Disk and BOOT Disk
    //         if(iso9660_mount(iso_disk_no) != 0){
    //             printf("[KMAIN] Failed to mount ISO9660 FS in Disk %d!\n", iso_disk_no);
    //         }
    //         printf("[KMAIN] Successfully mount ISO9660 FS in Disk %d.\n", iso_disk_no);
    //     }else if(disk.type == DISK_TYPE_AHCI_SATA ){
    //         // Mounting AHCI SATA Disk
    //         if(!fat32_mount(boot_disk_no, ESP_START_LBA)){
    //             printf("[KMAIN] Failed to Mount FAT32 FS at LBA: %d!\n", ESP_START_LBA);
    //             return;
    //         }
    //         printf("[KMAIN] Successfully Mount Disk %d.\n", boot_disk_no);
    //     }else{
    //         printf("[KMAIN] The Disk Type %d is not working currently!\n", disk.type);
    //     }
    // }

    // iso9660_test(iso_disk_no, "/BOOT/LIMINE.CON");

    // if(fat32_test(boot_disk_no, ESP_START_LBA)){
    //     printf("[KMAIN] FAT32 Test Success.\n");
    // }
    // printf("[KMAIN] Failed to test FAT32!\n");


    // vfs_test(boot_disk_no);

    if(!verify_installation(iso_disk_no, ESP_START_LBA)){
        if(format_disk_and_install(iso_disk_no, boot_disk_no)){
            printf("[KMAIN] Successfully Install KeblaOS in Disk %d.\n", boot_disk_no);
        }
    }else{
        disk_no = iso_disk_no;
        if(fat32_mount(DATA_PART_START_LBA)){
            printf("Successfully Mount Disk %d\n", iso_disk_no);
        }
    }
    
   
    test_time_functions();

    
    // mouse_init();

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












