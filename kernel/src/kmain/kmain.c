
/*
    Kernel.c
    Build Date  : 16-12-2024
    Last Update : 04-03-2026
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

#define MAIN_DISK_TOTAL_SECTORS 2097152         // 1GB = 1 * 1024 * 1024 * 1024 / 512

#define ESP_START_LBA 2048
#define ESP_SECTORS (100 * 1024 * 1024 / 512)   // 100 MB = 100 * 1024 * 1024 / 512

#define DATA_PART_START_LBA (ESP_START_LBA + ESP_SECTORS)
#define DATA_PART_SECTORS ((1 * 1024 * 1024 * 1024 / 512) - ESP_SECTORS - ESP_START_LBA)

int boot_disk_no = 0;    // The Disk no which boot KeblaOS
int main_disk_no = 1;    // The Main Disk Present in the System

extern int disk_no;      // The Disk Which have FAT32 Filesystem

bool install = true;    // This variable is set to true when the installer is running, otherwise it is false.



void kmain(){
    
    serial_init("Successfully Serial Printing initialized!\n");

    get_bootloader_info();
    init_vga();
    
    print_bootloader_info();

    get_set_memory();
    
    init_bs_cpu_core();
    
    // Initialize APIC and IOAPIC
    if(!has_apic()) printf("[Error] This System does not have APIC.\n");
        
    init_all_cpu_cores();    // Starts all CPU cores

    if(!pci_exists()){
        printf("[Error] This system do not have PCI!\n");
    }

    init_controllers();     // This function has PCI Scan

    // Detect and Initialize all Disks
    if(kebla_get_disks() <= 0){
        printf("No Disk Found!\n");
    }
    printf("[KMAIN] Total %d Disks Found.\n", disk_count);

    // Print detected disk type
    for(int disk=0; disk < disk_count; disk++){
        printf("[KMAIN] %d: Disk Type: %d\n", disk, find_disk_type(disk));
    }

    // print keblaos boot from where
    switch(find_disk_type(0)){
        case DISK_TYPE_SATAPI:
            printf("[KMAIN] KeblaOS is Booting from ISO Disk type.\n\n");
            break;
        case DISK_TYPE_AHCI_SATA:
            printf("[KMAIN] KeblaOS is Booting from AHCI SATA Disk type.\n\n");
            break;
        case DISK_TYPE_IDE_PATA:
            printf("[KMAIN] KeblaOS is Booting from IDE PATA Disk type.\n\n");
            break;
        default:
            printf("[KMAIN] KeblaOS is Booting from an Unknown Disk type.\n\n");
    }

    if(disk_count == 1 && disks[0].type != DISK_TYPE_SATAPI){
        boot_disk_no = 0;
        main_disk_no = 0;
        disk_no = 0;
    }else if(disk_count > 1){
        boot_disk_no = 0;
        main_disk_no = 1;
        disk_no = 1;
    }

    // Checking Installation in main_disk
    if(!verify_installation(main_disk_no, ESP_START_LBA)){
        
        printf("[KMAIN] KeblaOS is not installed in Disk %d\n", main_disk_no);
        if(install){
            if(uefi_install(boot_disk_no, main_disk_no, MAIN_DISK_TOTAL_SECTORS)){
                printf("[KMAIN] Successfully Install KeblaOS in Disk %d.\n", boot_disk_no);
            }
        }
    }else{
        printf("[KMAIN] KeblaOS is already installed in the Disk %d.\n", main_disk_no);
    }


    if(fat32_mount(DATA_PART_START_LBA)){
        printf("[KMAIN] Successfully Mount Disk %d of DATA Partition\n", main_disk_no);
    }



    // vfs_test(boot_disk_no);

    
   
    // test_time_functions();

    
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
    load_user_elf_and_jump();

    // acpi_poweroff();
    // acpi_reboot();

    // while(true){
    //     printf("Time: %d, Random Number: %d\n", get_apic_ticks(), rand());
    //     sleep_seconds(0, 10);
    // }

    // switch_to_core(2);
    

    halt_kernel();
}












