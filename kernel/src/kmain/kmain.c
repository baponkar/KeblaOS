
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

extern uint64_t fb0_width;
extern uint64_t fb0_height;
extern uint64_t fb0_pitch;
extern uint16_t fb0_bpp;



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

    // test_time_functions();

    // print_current_rtc_time();
    // sleep_seconds(0, 10);
    // printf("10 sec Sleep Test Over!\n");
    // print_current_rtc_time();


    for(int i=0; i<mass_storage_count; i++){
        if(!kebla_disk_init(i)){
            printf("Disk - %d initialization is failed!\n", i);
        }
    }
    printf("Total available Disks: %d\n", get_total_disks());
    

    printf("fb0_width = %d, fb0_height = %d, fb0_pitch = %d, fb0_bpp = %d \n", fb0_width, fb0_height, fb0_pitch, fb0_bpp);

    enable_mouse();
    mouse_init();

    clear_screen();
    


    ugui_test_1();
    // ugui_test_2();

    // int_syscall_test();

    // switch_to_core(3);

    // init_user_mode();

    // Load and parse kernel modules by using limine bootloader
    // get_kernel_modules_info();
    // print_kernel_modules_info();
    // load_user_elf_and_jump();

    halt_kernel();
}




