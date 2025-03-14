/*
Kernel.c
Build Date  : 16-12-2024
Last Update : 15-01-2025
Description : KeblaOS is a x86 architecture based 64 bit Operating System. Currently it is using Limine Bootloader.
Reference   : https://wiki.osdev.org/Limine
              https://github.com/limine-bootloader/limine-c-template
              https://wiki.osdev.org/Limine_Bare_Bones

*/

#include "../process/process.h" 
#include "../process/test_process.h"
#include "../acpi/acpi.h" // init_acpi
#include "../x86_64/interrupt/interrupt.h"
#include "../x86_64/interrupt/apic.h"
#include "../x86_64/interrupt/pic.h" // init_idt, test_interrupt
#include "../bootloader/ahci.h"
#include "../bootloader/pci.h"
#include "../bootloader/disk.h"
#include "../bootloader/cpu.h"
#include "../bootloader/trampoline.h"
#include "../bootloader/memory.h"
#include "../bootloader/firmware.h"
#include "../limine/limine.h" // bootloader info
#include "../bootloader/boot.h" // bootloader info
#include "../lib/stdio.h" // printf
#include "../util/util.h" // registers_t , halt_kernel
#include "../driver/vga/framebuffer.h"
#include "../driver/vga/vga_term.h" // vga_init, print_bootloader_info, print_memory_map, display_image
#include "../driver/image_data.h"
#include "../driver/io/serial.h"
#include "../x86_64/gdt/gdt.h" // init_gdt
#include "../mmu/pmm.h" // init_pmm, test_pmm
#include "../mmu/paging.h" // init_paging, test_paging
#include "../mmu/kmalloc.h" // test_kmalloc
#include "../mmu/vmm.h" // test_vmm
#include "../mmu/kheap.h" // init_kheap, test_kheap
#include "../driver/keyboard/keyboard.h" // initKeyboard
#include "../x86_64/timer/tsc.h"         // time stamp counter
#include "../x86_64/timer/rtc.h"        // RTC
#include "../x86_64/timer/pit_timer.h"  // init_timer
#include "../x86_64/timer/apic_timer.h" // apic timer
#include "../x86_64/timer/hpet_timer.h" // hpet timer
#include "../usr/shell.h"

#include "kernel.h"


extern process_t *current_process;
__attribute__((section(".data"))) uint8_t core_id = 1;  // Dummy value for now

void store_core_id() {
    *(volatile uint8_t*)0x9000 = core_id;
}

void kmain(){

    serial_init();
    get_bootloader_info();
    get_memory_info();
    vga_init();

    printf("%s - %s\n", OS_NAME, OS_VERSION);

    init_acpi();

    // Enabling GDT
    // init_gdt_bootstrap_cpu();
    init_all_gdt_tss();
    core_init(0);
    start_secondary_cores();
    


    // Memory management initialization
    init_pmm();
    init_paging();
    init_kheap();

    // Enabling interrupt
    init_bootstrap_cpu_interrupt();
    
    
    init_pic_interrupt();
    init_apic_interrupt();

    // init_core_cpu_interrupt(core_id);

    // lapic_send_ipi(1, 0x4500); // INIT IPI
    // for (volatile int i = 0; i < 100000; i++);
    // lapic_send_ipi(1, 0x4608); // SIPI with vector 0x08
    // for (volatile int i = 0; i < 100000; i++);
    // lapic_send_ipi(1, 0x4608); // Second SIPI




    // Timer initialization
    init_tsc();
    rtc_init();
    // init_hpet();
    init_pit_timer(100);    // Interrupt in 100 ms
    init_apic_timer(100);   // Interrupt in 100 ms
    

    get_cpu_info();
    print_cpu_info();
    // print_cpu_vendor();
    // print_cpu_brand();
    // printf("Logical Processor Count: %d\n", getLogicalProcessorCount());
    
    // detect_ahci();
    // init_ahci( 0xFEBD5000);
    // pci_scan();
    // get_disk_info();

    // test_interrupt();

    initKeyboard();

    init_processes();
    // print_all_threads_name(current_process);

    // if(has_fpu){
    //     enable_fpu();

    //     float flt = 23.67890f;
    //     printf("Test float: flt = %f\n", flt);
    // }

    uint64_t low_addr = 0x0000000000400000; 

    if (is_user_page(low_addr)) {
        printf("0x0000000000400000 : User page\n");
    } else {
        printf("0x0000000000400000 : Kernel page\n");
    }
    
    halt_kernel();
}













