
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


extern ring_buffer_t* keyboard_buffer;          // To get the keyboard input



void kmain(){

    serial_init("Successfully Serial Printing initialized!\n");

    get_bootloader_info();
    init_vga();
    
    // print_bootloader_info();

    get_set_memory();
        
    init_bs_cpu_core();

    // Initialize APIC and IOAPIC
    if(has_apic()){
        init_all_cpu_cores();    // Starts all CPU cores
    }else{
        printf("[Error] This System does not have APIC.\n");
    }

    pci_scan();

    // print_current_time();
    // test_time_functions();

    // sleep_seconds(0, 1);
    // printf("Sleep Test Over!\n");
    // print_current_time();


    for(int i=0; i<mass_storage_count; i++){
        if(!kebla_disk_init(i)){
            printf("Disk - %d initialization is failed!\n", i);
        }
    }
    printf("Total available Disks: %d\n", get_total_disks());
    


    // int_syscall_test();

    // switch_to_core(3);

    // init_user_mode();

    // Load and parse kernel modules by using limine bootloader
    // get_kernel_modules_info();
    // print_kernel_modules_info();
    load_user_elf_and_jump();


    halt_kernel();
}




