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

#include "../FatFs-R0.15b/fatfs.h"
#include "../usr/switch_to_user.h"
#include "../usr/load_and_parse_elf.h"

#include "../memory/vmm.h"
#include "../driver/vga/vga_gfx.h"
#include "../driver/vga/framebuffer.h"
#include "../process/process.h" 
#include "../process/test_process.h"
#include "../sys/acpi/acpi.h"                   // init_acpi
#include "../sys/acpi/descriptor_table/mcfg.h"
#include "../sys/acpi/descriptor_table/madt.h"

// File System
#include "../fs/kfs.h"                      // Kebla File System
#include "../fs/fat16.h"                    // FAT16 File System
#include "../fs/fat32.h"                    // FAT32 File System
#include "../vfs/vfs.h"

#include "../arch/interrupt/apic/apic_interrupt.h"
#include "../arch/interrupt/apic/apic.h"
#include "../arch/interrupt/apic/ioapic.h"
#include "../arch/interrupt/pic/pic.h"      // init_idt, test_interrupt
#include "../arch/interrupt/pic/pic_interrupt.h"

#include "../driver/disk/ahci/ahci.h"
#include "../sys/pci/pci.h"
#include "../bootloader/sysinfo.h"
#include "../sys/cpu/cpu.h"                 // target_cpu_task, switch_to_core
#include  "../sys/cpu/cpuid.h"              // get_cpu_count, get_cpu_info
#include "../memory/detect_memory.h"
#include "../bootloader/firmware.h"
#include "../../../limine-9.2.3/limine.h"   // bootloader info
#include "../bootloader/boot.h"             // bootloader info
#include "../lib/stdio.h"                   // printf
#include "../lib/string.h"
#include "../util/util.h"                   // registers_t , halt_kernel
#include "../driver/vga/framebuffer.h"
#include "../driver/vga/vga_term.h"         // vga_init, print_bootloader_info, print_memory_map, display_image
#include "../driver/image_data.h"
#include "../driver/io/serial.h"
#include "../arch/gdt/gdt.h"                // init_gdt
#include "../arch/gdt/tss.h"

#include "../memory/pmm.h"                  // init_pmm, test_pmm
#include "../memory/paging.h"               // init_paging, test_paging
#include "../memory/kmalloc.h"              // test_kmalloc
#include "../memory/vmm.h"                  // test_vmm
#include "../memory/kheap.h"                // test_kheap

#include "../sys/timer/tsc.h"               // time stamp counter
#include "../sys/timer/rtc.h"               // RTC
#include "../sys/timer/pit_timer.h"         // init_timer
#include "../sys/timer/apic_timer.h"        // apic timer
#include "../sys/timer/hpet_timer.h"        // hpet timerzz
#include "../kshell/kshell.h"               // Kernel shell
#include "../driver/keyboard/ring_buffer.h" // Hold keyboard input
#include "../driver/mouse/mouse.h"          // mouse driver
#include "../syscall/syscall_manager.h"
#include "../syscall/int_syscall_manager.h"
#include "../driver/vga/color.h"
#include "../arch/interrupt/irq_manage.h"

#include "../usr/usr_shell.h"

#include "../sys/cpu/smp.h"

#include "kmain.h"


// Found pci devices from pci_scan
extern pci_device_t mass_storage_controllers[16];   // Array to store detected mass storage devices
extern size_t mass_storage_count;                   // Counter for mass storage devices

extern pci_device_t network_controllers[16];        // Array to store detected network controllers
extern size_t network_controller_count;             // Counter for network controllers

extern pci_device_t wireless_controllers[16];       // Array to store detected wireless controllers
extern size_t wireless_controller_count;            // Counter for wireless controllers

extern ring_buffer_t* keyboard_buffer;              // To get the keyboard input


void kmain(){

    serial_init("Successfully Serial Printing initialized!\n");

    get_bootloader_info();
    vga_init();
    print_bootloader_info();
        
    init_bs_cpu_core();

    // Initialize APIC and IOAPIC
    if(has_apic()){
        init_all_cpu_cores();    // Starts all CPU cores
    }else{
        printf("[Error] This System does not have APIC.\n");
    }

    asm volatile("int $49");

    // init_syscall(0);
    // test_syscall();


    pci_scan();
    
    uint32_t bar5 = mass_storage_controllers[0].base_address_registers[5];  // Found from pci scan 0xFEBD5000
    HBA_PORT_T* port = get_ahci_port(bar5);
    ahci_identify(port);

    uint8_t dump[512];
    ahci_read(port, 0, 0, 1, (uint16_t *)&dump);
    printf("Boot sector signature: %x %x\n", dump[510], dump[511]);         // should be 0x55 0xAA


    fatfs_init(port);
    // test_fatfs();

    // switch_to_core(3);

    // start_kshell();

    // test_vfs();
    // system_call_test();

    printf("Testing Keyboard Interrut\n");
    asm volatile("int $33");

    // Load and parse kernel modules by using limine bootloader
    get_kernel_modules_info();
    // print_kernel_modules_info();
    load_user_elf_and_jump();

    // init_user_mode();

    halt_kernel();
}




