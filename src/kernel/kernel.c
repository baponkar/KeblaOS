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

#include "../process/process.h" 
#include "../process/test_process.h"
#include "../acpi/acpi.h" // init_acpi
#include "../x86_64/interrupt/interrupt.h"
#include "../x86_64/interrupt/apic.h"
#include "../x86_64/interrupt/pic.h" // init_idt, test_interrupt
#include "../ahci/ahci.h"
#include "../pci/pci.h"
#include "../disk/disk.h"
#include "../cpu/cpu.h" // target_cpu_task, switch_to_core
#include "../bootloader/trampoline.h"
#include "../mmu/detect_memory.h"
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
#include "../mmu/umalloc.h" // test_umalloc
#include "../mmu/vmm.h" // test_vmm
#include "../mmu/kheap.h" // test_kheap
#include "../mmu/uheap.h" // test_uheap
#include "../driver/keyboard/keyboard.h" // initKeyboard
#include "../x86_64/timer/tsc.h"         // time stamp counter
#include "../x86_64/timer/rtc.h"        // RTC
#include "../x86_64/timer/pit_timer.h"  // init_timer
#include "../x86_64/timer/apic_timer.h" // apic timer
#include "../x86_64/timer/hpet_timer.h" // hpet timer
#include "../usr/shell.h"

#include "kernel.h"

extern uint64_t V_UMEM_LOW_BASE;

void kmain(){

    serial_init();
    get_bootloader_info();
    get_memory_info();
    vga_init();

    printf("%s - %s\n", OS_NAME, OS_VERSION);
    
    init_acpi();

    // Enabling GDT
    init_all_gdt_tss();
    core_init(0);
    init_core_cpu_interrupt(0);

    init_pic_interrupt();
    init_apic_interrupt();

    // Memory management initialization
    init_pmm();
    init_paging();
    
    // Timer initialization
    init_tsc();
    rtc_init();
    init_pit_timer(100);    // Interrupt in 100 ms
    init_apic_timer(100);   // Interrupt in 100 ms
    start_hpet();

    // pci_scan();

    initKeyboard();

    printf("-------------------------------------------------------------------------------\n");
    
    // init_processes();

    detect_ahci();
    get_disk_info();
    test_disk_write_read();


    halt_kernel();
}













