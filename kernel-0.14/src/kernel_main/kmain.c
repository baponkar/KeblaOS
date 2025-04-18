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


#include "../memory/vmm.h"
#include "../driver/vga/vga_gfx.h"
#include "../driver/vga/framebuffer.h"
#include "../process/process.h" 
#include "../process/test_process.h"
#include "../acpi/acpi.h"               // init_acpi
#include "../acpi/descriptor_table/mcfg.h"
#include "../acpi/descriptor_table/madt.h"
#include "../x86_64/interrupt/multi_core_interrupt.h"
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
#include "../../../limine-8.6.0/limine.h"// bootloader info
#include "../bootloader/boot.h"         // bootloader info
#include "../lib/stdio.h"               // printf
#include "../lib/string.h"
#include "../util/util.h"               // registers_t , halt_kernel
#include "../driver/vga/framebuffer.h"
#include "../driver/vga/vga_term.h"     // vga_init, print_bootloader_info, print_memory_map, display_image
#include "../driver/image_data.h"
#include "../driver/io/serial.h"
#include "../x86_64/gdt/gdt.h"           // init_gdt
#include "../x86_64/gdt/tss.h"
#include "../memory/pmm.h"               // init_pmm, test_pmm
#include "../memory/paging.h"            // init_paging, test_paging
#include "../memory/kmalloc.h"           // test_kmalloc
#include "../memory/umalloc.h"           // test_umalloc
#include "../memory/vmm.h"               // test_vmm
#include "../memory/kheap.h"             // test_kheap
#include "../memory/uheap.h"             // test_uheap
// #include "../driver/keyboard/keyboard.h" // initKeyboard
#include "../x86_64/timer/tsc.h"         // time stamp counter
#include "../x86_64/timer/rtc.h"         // RTC
#include "../x86_64/timer/pit_timer.h"   // init_timer
#include "../x86_64/timer/apic_timer.h"  // apic timer
#include "../x86_64/timer/hpet_timer.h"  // hpet timerzz
#include "../kshell/kshell.h"
#include "../kshell/ring_buffer.h"
#include "../driver/mouse/mouse.h"       // mouse driver
#include "../usr/switch_to_user.h"
#include "../syscall/syscall_manager.h"
#include "../syscall/int_syscall_manager.h"
#include "../usr/load_and_parse_elf.h"
#include "../file_system/fat32.h"       // fat32_init
#include "../file_system/fs.h"          // test_file_operations()
#include "../driver/vga/color.h"
#include "../x86_64/interrupt/irq_manage.h"

#include "../usr/usr_shell.h"

#include "kmain.h"

extern uint64_t V_KMEM_LOW_BASE;
extern uint64_t V_KMEM_UP_BASE;

extern uint64_t V_UMEM_LOW_BASE;
extern uint64_t V_UMEM_UP_BASE;

extern uint64_t PHYSICAL_TO_VIRTUAL_OFFSET;     // 0xFFFFFFFF010CA000


extern ahci_controller_t sata_disk;             // Detecting by pci scan
extern ahci_controller_t network_controller;    // Detecting by pci scan

extern void restore_cpu_state(registers_t* registers);

extern tss_t tss;


void print_test(){
    printf("This is a test function\n");
}

void kmain(){

    serial_init();
    get_bootloader_info();
    vga_init();
    printf("[Info] %s - %s\nBuild starts on: %s, Last Update on: %s\n",
        OS_NAME, OS_VERSION, BUILD_DATE, LAST_UPDATE);
    get_set_memory();
    get_smp_info();

    
    
    if(has_apic()){
        disable_pic();
        gdt_tss_init();
        start_bootstrap_cpu_core();                 // Enabling GDT, TSS, Interrupt and APIC Timer for bootstrap core
        
        // Interrupt Based System Call
        int_syscall_init();

        // Memory management initialization
        init_pmm();
        init_paging();

        // if(has_apic()){
        //     set_ap_stacks(1, 3);                 // Initialize stacks for other cores
        //     start_secondary_cpu_cores(1, 3);     // Enabling GDT, TSS, Interrupt and APIC Timer for other cores
        // }

    }else{
        pic_irq_remap();
        // init_bootstrap_gdt_tss(0);

        init_pic_interrupt();
        init_pit_timer();
    }

    
    if(has_fpu()){
        enable_fpu_and_sse();
    }

    pci_scan();
    
 
    init_user_mode();

    halt_kernel();
}













