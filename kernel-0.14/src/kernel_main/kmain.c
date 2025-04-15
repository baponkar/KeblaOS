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
#include "../x86_64/timer/hpet_timer.h"  // hpet timer
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
    get_memory_info();
    vga_init();

    get_cpu_info();

    printf("[Info] %s - %s\n", OS_NAME, OS_VERSION);
    
    if(has_apic()){
        disable_pic();
        gdt_tss_init();
        printf("Going to initialize paging\n");
        start_bootstrap_cpu_core();             // Enabling GDT, TSS, Interrupt and APIC Timer for bootstrap core
        
        // print_gdt_entry(0);     // Null Descriptor
        // print_gdt_entry(0x08);  // Kernel code segment
        // print_gdt_entry(0x10);  // Kernel data segment
        // print_gdt_entry(0x1B);  // User code segment
        // print_gdt_entry(0x23);  // User data segment
        // print_gdt_entry(0x28);  // TSS segment
        // extern tss_t tss;
        // print_tss(&tss);

    }else{
        pic_irq_remap();
        // init_bootstrap_gdt_tss(0);

        init_pic_interrupt();
        init_pit_timer();
    }

    
    // Memory management initialization
    init_pmm();
    init_paging();

    // if(has_apic()){
    //     set_ap_stacks(1, 3);                 // Initialize stacks for other cores
    //     start_secondary_cpu_cores(1, 3);     // Enabling GDT, TSS, Interrupt and APIC Timer for other cores
    // }

    if(has_fpu()){
        enable_fpu_and_sse();
    }

    pci_scan();
    
    // Test AHCI drivers for a successful read
    HBA_MEM_T* abar = (HBA_MEM_T*) sata_disk.abar;
    probePort(abar);

    // test_ahci(sata_disk);

    // fat32_run_tests((HBA_PORT_T *)&abar->ports[0]);

    mouse_init();

    // MSR Based System Call
    // init_syscall();
    // init_user_mode();
    // _syscall(1, (uint64_t)(char *)"Hello\n", 0);
    

    // Interrupt Based System Call
    int_syscall_init();

    // test_file_operations(abar);
    // test_directory_operations(abar);
    // list_root_dir();

    // start_kshell();

    // get_kernel_modules_info();
    // print_kernel_modules_info();
    // load_user_elf_and_jump();



    
    print_gdt_entry(0);     // Null Descriptor
    print_gdt_entry(0x08);  // Kernel code segment
    print_gdt_entry(0x10);  // Kernel data segment
    print_gdt_entry(0x1B);  // User code segment
    print_gdt_entry(0x23);  // User data segment
    print_gdt_entry(0x28);  // TSS segment
    print_tss((tss_t *)&tss);            
    printf("Current stack address: %x\n", read_rsp());
    printf("Current rip address: %x\n", read_rip());
    printf("Current rflags address: %x\n", read_rflags());
    init_user_mode();
    // This won't be reached if successful
    printf("Failed to switch to user mode!\n");


    halt_kernel();
}













