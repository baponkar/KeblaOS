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
#include "../fs/kfs.h"      // Kebla File System
#include "../fs/fat16.h"    // FAT16 File System
#include "../fs/fat32.h"    // FAT32 File System

#include "../arch/interrupt/apic/apic_interrupt.h"
#include "../arch/interrupt/apic/apic.h"
#include "../arch/interrupt/apic/ioapic.h"
#include "../arch/interrupt/pic/pic.h"    // init_idt, test_interrupt
#include "../arch/interrupt/pic/pic_interrupt.h"

#include "../sys/ahci/ahci.h"
#include "../sys/pci/pci.h"
#include "../bootloader/sysinfo.h"
#include "../sys/cpu/cpu.h"                 // target_cpu_task, switch_to_core
#include  "../sys/cpu/cpuid.h"              // get_cpu_count, get_cpu_info
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
#include "../arch/gdt/gdt.h"           // init_gdt
#include "../arch/gdt/tss.h"
#include "../memory/pmm.h"               // init_pmm, test_pmm
#include "../memory/paging.h"            // init_paging, test_paging
#include "../memory/kmalloc.h"           // test_kmalloc
#include "../memory/vmm.h"               // test_vmm
#include "../memory/kheap.h"             // test_kheap
#include "../sys/timer/tsc.h"         // time stamp counter
#include "../sys/timer/rtc.h"         // RTC
#include "../sys/timer/pit_timer.h"   // init_timer
#include "../sys/timer/apic_timer.h"  // apic timer
#include "../sys/timer/hpet_timer.h"  // hpet timerzz
#include "../kshell/kshell.h"
#include "../kshell/ring_buffer.h"
#include "../driver/mouse/mouse.h"       // mouse driver
#include "../syscall/syscall_manager.h"
#include "../syscall/int_syscall_manager.h"
#include "../driver/vga/color.h"
#include "../arch/interrupt/irq_manage.h"

#include "../usr/usr_shell.h"

#include "../sys/cpu/smp.h"

#include "kmain.h"

// Found from detect_memory
extern uint64_t HHDM_OFFSET; 
extern volatile uint64_t phys_mem_head;


// Found pci devices from pci_scan
extern pci_device_t mass_storage_controllers[16];  // Array to store detected mass storage devices
extern size_t mass_storage_count;               // Counter for mass storage devices
extern pci_device_t network_controllers[16];       // Array to store detected network controllers
extern size_t network_controller_count;        // Counter for network controllers
extern pci_device_t wireless_controllers[16];      // Array to store detected wireless controllers
extern size_t wireless_controller_count;       // Counter for wireless controllers

extern void restore_cpu_state(registers_t* registers);

extern tss_t tss;



void kmain(){

    serial_init("phys_mem_head : %d\n", phys_mem_head);

    get_bootloader_info();
    vga_init();
    print_bootloader_info();
    printf("[%s - %s\n[Info] Build starts on: %s, Last Update on: %s]\n",
        OS_NAME, OS_VERSION, BUILD_DATE, LAST_UPDATE);
        
    init_bs_cpu_core();

    // Initialize APIC and IOAPIC
    if(has_apic()){
        init_all_cpu_cores();    // Starts all CPU cores
    }else{
        printf("[Error] This System does not have APIC.\n");
    }

    // printf("\n[Hello from CPU %d (BSP)]\n", 0);

    // pci_scan();

    // uint32_t bar5 = mass_storage_controllers[0].base_address_registers[5]; // Found from pci scan 0xFEBD5000
    // HBA_MEM_T* abar = (HBA_MEM_T*) bar5;
    // HBA_PORT_T* port = (HBA_PORT_T*) &abar->ports[0];

    // ahci_identify(port);
    // test_ahci(abar);

    // fat32_init(port);
    // fat32_run_tests(port);


    // switch_to_core(3);

    // start_kshell();

    // printf("%s\n", *syscall_test(INT_SYSCALL_READ));    // Test Read System Call
    // printf("%s\n", *syscall_test(INT_SYSCALL_PRINT));   // Test Print System Call
    // printf("%s\n", *syscall_test(INT_SYSCALL_EXIT));    // Test Exit System Call

    // syscall_test(INT_SYSCALL_PRINT);   // Test Print System Call
    // syscall_test(INT_SYSCALL_READ);   // Test Print System Call
    // syscall_test(INT_SYSCALL_EXIT);    // Test Exit System Call

    // Load and parse kernel modules by using limine bootloader
    // get_kernel_modules_info();
    // print_kernel_modules_info();
    // load_user_elf_and_jump();

    // loading usermode function
    init_user_mode();

    halt_kernel();
}

void test_syscall_entry(uint64_t rcx, uint64_t r11){
    printf("[Syscall_Entry] rcx: %x, r11: %x\n", rcx, r11);
}













