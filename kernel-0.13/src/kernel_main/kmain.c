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
#include "../driver/vga/vga_gfx.h"
#include "../driver/vga/framebuffer.h"
#include "../driver/vga/my_lvgl.h"
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
#include "../../../limine-8.6.0/limine.h"           // bootloader info
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
#include "../kshell/kshell.h"
#include "../kshell/ring_buffer.h"
#include "../driver/mouse/mouse.h"       // mouse driver
#include "../syscall/switch_to_user.h"


#include "../file_system/fs.h"
#include "../file_system/fat32.h"       // fat32_init
#include "../driver/vga/color.h"
#include "../driver/vga/my_lvgl.h"
#include "../../../lvgl-9.2.2/lvgl.h"


#include "kmain.h"


extern ahci_controller_t sata_disk;             // Detecting by pci scan
extern ahci_controller_t network_controller;    // Detecting by pci scan


void kmain(){

    serial_init();
    get_bootloader_info();
    get_memory_info();
    vga_init();

    get_cpu_info();

    printf("[Info] %s - %s\n", OS_NAME, OS_VERSION);
    
    if(has_apic()){
        disable_pic();
        start_bootstrap_cpu_core();             // Enabling GDT, TSS, Interrupt and APIC Timer for bootstrap core
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
    //     set_ap_stacks(1, 3);                 // Initialize stacks for other cores
    //     start_secondary_cpu_cores(1, 3);     // Enabling GDT, TSS, Interrupt and APIC Timer for other cores
    // }

    if(has_fpu()){
        enable_fpu_and_sse();
    }

    pci_scan();
    
    // Test AHCI drivers for a successful read
    HBA_MEM_T* host = (HBA_MEM_T*) sata_disk.abar;
    probePort(host);

    test_ahci(sata_disk);

    fat32_init((HBA_PORT_T *)&host->ports[0]);
    fat32_read_root_dir();
    fat32_run_tests((HBA_PORT_T *)&host->ports[0]);

    mouse_init();

    printf("\n");
    draw_horizontal_line(get_cursor_pos_x(), get_cursor_pos_y(), get_fb_width(), COLOR_WHITE);

    // apic_delay(500);   // Delay for 0.5 seconds to show boot messages
    // clear_screen();
    // load_image_with_animation(
    //     (get_fb_width() - KEBLAOS_WIDTH) / 2,
    //     (get_fb_height() - KEBLAOS_HEIGHT) / 2,  
    //     KeblaOS, 
    //     KEBLAOS_WIDTH, 
    //     KEBLAOS_HEIGHT
    // );

    
    
    lvgl_init();
    // create_gui();
    create_gui_1();
    // create_multiple_windows();
    // create_gui_animated();

    while (1) {
        lv_timer_handler();   // Process LVGL tasks
        apic_delay(5);        // Delay ~5 milliseconds (or your timer delay function)
    }
    
    // start_kshell();

    halt_kernel();
}













