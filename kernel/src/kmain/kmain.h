
#pragma once

// The following header files are present in gcc even in -ffreestanding mode
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <iso646.h>
#include <stdatomic.h>
#include <cpuid.h>

// User's functions
#include "../usr/switch_to_user.h"
#include "../usr/load_and_parse_elf.h"

// Process-Thread
#include "../process/process.h" 
#include "../process/test_process.h"


// Memory Management
#include "../memory/pmm.h"                  // init_pmm, test_pmm
#include "../memory/paging.h"               // init_paging, test_paging
#include "../memory/kmalloc.h"              // test_kmalloc
#include "../memory/vmm.h"                  // test_vmm
#include "../memory/kheap.h"                // test_kheap
#include "../memory/detect_memory.h"

// Disk Driver
#include "../driver/disk/ahci/ahci.h"       // AHCI Driver
#include "../driver/disk/disk.h"            // Disk structure and functions
#include "../driver/disk/inspect_disk.h"


// File System
#include "../fs/fatfs_wrapper.h"
#include "../vfs/vfs.h"                         // Virtual FIle System 
#include "../vfs/tree.h"


// Interrupt
#include "../sys/acpi/acpi.h"                   // init_acpi
#include "../sys/acpi/descriptor_table/mcfg.h"
#include "../sys/acpi/descriptor_table/madt.h"
#include "../arch/interrupt/apic/apic_interrupt.h"
#include "../arch/interrupt/apic/apic.h"
#include "../arch/interrupt/apic/ioapic.h"
#include "../arch/interrupt/irq_manage.h"
#include "../arch/interrupt/pic/pic.h"          // init_idt, test_interrupt
#include "../arch/interrupt/pic/pic_interrupt.h"


// Hardware Drivers
#include "../driver/pci/pci.h"                  // PCI Driver
#include "../driver/keyboard/ring_buffer.h"     // Hold keyboard input
#include "../driver/mouse/mouse.h"              // mouse driver
#include "../driver/io/serial.h"

// VGA Drivers
#include "../driver/vga/framebuffer.h"
#include "../driver/vga/vga_term.h"             // vga_init, print_bootloader_info, print_memory_map, display_image
#include "../driver/vga/data/image_data.h"
#include "../driver/vga/color.h"
#include "../driver/vga/vga.h"


// System Info
#include "../bootloader/sysinfo.h"
#include "../sys/cpu/cpu.h"                 // target_cpu_task, switch_to_core
#include  "../sys/cpu/cpuid.h"              // get_cpu_count, get_cpu_info
#include "../bootloader/firmware.h"
#include "../sys/cpu/smp.h"

// Bootloader
#include "../bootloader/boot.h"                     // bootloader info

// Standard C Library
#include "../lib/assert.h"
#include "../lib/ctype.h"
#include "../lib/limit.h"
#include "../lib/math.h"
#include "../lib/stdio.h"                   // printf
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/time.h"
#include "../lib/libc.h"


// Utility Functions
#include "../util/util.h"                   // registers_t , halt_kernel
#include "../arch/gdt/gdt.h"                // init_gdt
#include "../arch/gdt/tss.h"

// Timer
#include "../sys/timer/tsc.h"               // time stamp counter
#include "../sys/timer/rtc.h"               // RTC
#include "../sys/timer/pit_timer.h"         // init_timer
#include "../sys/timer/apic_timer.h"        // apic timer
#include "../sys/timer/hpet_timer.h"        // hpet timer

// System Call
#include "../syscall/int_syscall_manager.h"     // Interrupt Based System Call
#include "../syscall/syscall_manager.h"         // MSR Based System Call



#include "../kshell/kshell.h"               // Kernel shell


// =============================Externel Library=========================================

// Limine Bootloader
#include "../../../ext_lib/limine-9.2.3/limine.h"   // bootloader info

// FatFs Library
#include "../../../ext_lib/FatFs-R0.15b/source/ff.h"
#include "../../../ext_lib/FatFs-R0.15b/source/diskio.h"

// tiny-regex-c
#include "../../../ext_lib/tiny-regex-c/re.h"
#include "../../../ext_lib/tiny-regex-c/re_test.h"

// lvgl
#include "../../../ext_lib/lvgl-9.3.0/lvgl.h"
#include "../../../ext_lib/lvgl-9.3.0/lvgl_fb.h"

// =======================================================================================

#define OS_NAME "KeblaOS"
#define OS_VERSION "1.1"
#define BUILD_DATE "17/07/2025"
#define LAST_UPDATE "07/09/2025"


extern uint8_t core_id;
uint32_t get_lapic_id();
void route_keyboard_irq_to_bsp() ;
void kmain();




