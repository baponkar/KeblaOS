
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

    // pci_scan();
    init_controllers();     // This have PCI Scan


    // Test Disks
    kebla_disk_init(0);
    kebla_disk_init(1);
    kebla_disk_init(2);
    
    // fatfs_test(0);
    // iso9660_disk_test(2);

    install_kebla_os(2, 0);


    // install_kebla_os(2, 0);
    // test_iso9660(disks[2].context);

    // Example usage
    // installer_context_t *installer = installer_init(disks[2].context, 0, example_progress_callback, example_status_callback);

    // if (installer) {
    //     bool success = install_operating_system(installer, true); // true = format disk
    //     installer_cleanup(installer);
    // }
    
    // disk_test(0);
    // vfs_test(0);
    // init_nvme();
    // for(int i=0; i<disk_count; i++){
    //     printf(" Disk-%d, Type-%d\n", i, disks[i].type);
    //     detect_partition_table(i);
    //     printf(" Detect Filesystem %d in Disk-0\n", detect_filesystem(i));
    // }
    
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












