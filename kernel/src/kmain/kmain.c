/*
Kernel.c
Build Date  : 16-12-2024
Last Update : 15-01-2025
Description : KeblaOS is a x86 architecture based 64 bit Operating System. Currently it is using Limine Bootloader.
Reference   : https://wiki.osdev.org/Limine
              https://github.com/limine-bootloader/limine-c-template
              https://wiki.osdev.org/Limine_Bare_Bones
              https://wiki.osdev.org/SSE
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

    kebla_disk_init(iso_disk_no);   // Initialize first Disk either ISO or SATA/NVMe Disk
    // kebla_disk_init(boot_disk_no);
    // kebla_disk_test(boot_disk_no);

    for(int i = 0; i < disk_count; i++){
        Disk disk = disks[i];

        printf("disk: %d, type: %d\n", i, disk.type);
    }
    

    if(disks[iso_disk_no].type != DISK_TYPE_SATAPI ||  disks[iso_disk_no].type == DISK_TYPE_AHCI_SATA || disks[iso_disk_no].type == DISK_TYPE_NVME){
        boot_disk_no = iso_disk_no;

        init_user_space(boot_disk_no, boot_ld, user_ld);
        
    }else{
        if(is_os_installed(boot_disk_no, boot_ld) == false){
            if(uefi_install(iso_disk_no, boot_disk_no) != 0){
                printf("Failed to UEFI Install KeblaOS in Disk %d failed!\n", boot_disk_no);
            }else{
                printf("Successfully KeblaOS is Installed(UEFI) in Disk %d:%d\n", boot_disk_no, boot_ld);
            }

            if(create_user_dirs(boot_disk_no) != 0){
                printf("Failed to Create User Directories and Files in %d:%d\n", boot_disk_no, user_ld);
            }else{
                printf("Successfully Created directories and files in Disk %d:%d.\n", boot_disk_no, user_ld);
            }
        }
    }
    
    
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

    int_syscall_init();
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












