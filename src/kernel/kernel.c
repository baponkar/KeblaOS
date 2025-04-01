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
#include "../acpi/acpi.h"               // init_acpi
#include "../acpi/descriptor_table/mcfg.h"
#include "../acpi/descriptor_table/madt.h"
#include "../x86_64/interrupt/interrupt.h"
#include "../x86_64/interrupt/apic.h"
#include "../x86_64/interrupt/ioapic.h"
#include "../x86_64/interrupt/pic.h"    // init_idt, test_interrupt
#include "../ahci/ahci.h"
#include "../pci/pci.h"
#include "../disk/disk.h"
#include "../cpu/cpu.h"                 // target_cpu_task, switch_to_core
#include  "../cpu/cpuid.h"              // get_cpu_count, get_cpu_info
#include "../memory/detect_memory.h"
#include "../bootloader/firmware.h"
#include "../limine/limine.h"           // bootloader info
#include "../bootloader/boot.h"         // bootloader info
#include "../lib/stdio.h"               // printf
#include "../lib/string.h"
#include "../util/util.h"               // registers_t , halt_kernel
#include "../driver/vga/framebuffer.h"
#include "../driver/vga/vga_term.h"     // vga_init, print_bootloader_info, print_memory_map, display_image
#include "../driver/image_data.h"
#include "../driver/io/serial.h"
#include "../x86_64/gdt/gdt.h"           // init_gdt
#include "../memory/pmm.h"               // init_pmm, test_pmm
#include "../memory/paging.h"            // init_paging, test_paging
#include "../memory/kmalloc.h"           // test_kmalloc
#include "../memory/umalloc.h"           // test_umalloc
#include "../memory/vmm.h"               // test_vmm
#include "../memory/kheap.h"             // test_kheap
#include "../memory/uheap.h"             // test_uheap
#include "../driver/keyboard/keyboard.h" // initKeyboard
#include "../x86_64/timer/tsc.h"         // time stamp counter
#include "../x86_64/timer/rtc.h"         // RTC
#include "../x86_64/timer/pit_timer.h"   // init_timer
#include "../x86_64/timer/apic_timer.h"  // apic timer
#include "../x86_64/timer/hpet_timer.h"  // hpet timer
#include "../usr/shell.h"
#include "../usr/ring_buffer.h"
#include "../usr/switch_to_user.h"
#include "../usr/syscall.h"
#include "../file_system/fs.h"
#include "../driver/vga/color.h"

#include "kernel.h"

extern ahci_controller_t sata_disk;


void kmain(){

    serial_init();
    get_bootloader_info();
    get_memory_info();
    vga_init();

    get_cpu_info();

    printf("%s - %s\n", OS_NAME, OS_VERSION);
    
    if(has_apic()){
        disable_pic();
        start_bootstrap_cpu_core();         // Enabling GDT, TSS, Interrupt and APIC Timer for bootstrap core
    }else{
        pic_irq_remap();
        init_bootstrap_gdt_tss(0);
        init_pic_interrupt();
        init_pit_timer();
    }
    
    // Memory management initialization
    init_pmm();
    init_paging();

    // if(has_apic()){
    //     set_ap_stacks(1, 3);                // Initialize stacks for other cores
    //     start_secondary_cpu_cores(1, 3);    // Enabling GDT, TSS, Interrupt and APIC Timer for other cores
    // }

    printf("--------------------------------------\n");

    // start_shell();

    pci_scan();
    ahci_init();

    if (sata_disk.abar != 0) {

        char *buffer = (char *) kmalloc_a(512, 1);
        memset(buffer, 0, 512); // Clearing buffer

        if (ahci_read(0, 1, (void *)buffer) == 0) { 
            printf("Disk Read Successful!\n");
        
            // Check MBR signature (last 2 bytes of sector)
            if (buffer[510] == 0x55 && buffer[511] == 0xAA) {
                printf("Valid MBR Signature found!\n");
            } else {
                printf("No MBR found. Disk may be empty or unformatted.\n");
            }
        } else {
            printf("AHCI Read Failed!\n");
        }

        char *write_buffer = (char *) kmalloc_a(512, 1);
        memset(write_buffer, 'A', 512);  // Fill buffer with 'A'

        if (ahci_write(1, 1, write_buffer) == 0) {
            printf("AHCI Write Successful at LBA 1!\n");
        } else {
            printf("AHCI Write Failed!\n");
        }

        char *read_buffer = (char *) kmalloc_a(512, 1);
        memset(read_buffer, 0, 512);

        if (ahci_read(1, 1, read_buffer) == 0) {
            if (memcmp(write_buffer, read_buffer, 512) == 0) {
                printf("Write Verification Successful! Data matches. %x\n", *write_buffer);
            } else {
                printf("Write Verification Failed! Data does not match.\n");
            }
        } else {
            printf("Failed to read back written data.\n");
        }
        printf("Write_buffer[%d]: %x\n", 1,  write_buffer[1]);
        printf("Read_buffer[%d]: %x\n", 1, read_buffer[1]);
    }

    uint64_t test_va = (uint64_t) kheap_alloc(0x4000);
    uint64_t test_pa = vir_to_phys(test_va);
    printf("VA: %x => PA: %x\n", test_va, test_pa);

    halt_kernel();
}













