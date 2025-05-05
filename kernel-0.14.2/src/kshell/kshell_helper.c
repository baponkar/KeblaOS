/*
This file is part of the KSH shell.
Copyright (C) 2023 KeblaOS Project
Build Date: 2025-04-05
*/

#include "../kernel_main/kmain.h" // Kernel main header
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"
#include "../process/process.h"
#include "../process/thread.h"

#include "../memory/detect_memory.h" // Memory management functions

#include "../bootloader/boot.h" // Bootloader information
#include "../bootloader/firmware.h" // Firmware information

#include "../cpu/cpuid.h" // CPU information functions

#include "kshell_helper.h"

extern 

void print_os_features(){
    printf("\n");
    printf("%s-%s\n",  OS_NAME, OS_VERSION);
    printf("Architecture : x86_64.\n");
    printf(" 1. Limine Bootloading.(Ver. 0.6)\n");
    printf(" 2. GDT initialization(SMP Support).\n");
    printf(" 3. TSS initialization(SMP Support).\n");
    printf(" 3. VGA Graphics Driver.\n");
    printf(" 4. IDT initialization(SMP Support).\n");
    printf(" 5. Keyboard driver initialization.\n");
    printf(" 6. PIT Timer initialization.\n");
    printf(" 7. Basic Kernel Shell\n");
    printf(" 8. Memory Management Unit(Kheap, PMM, 4 Level Paging)\n");
    printf(" 7. Standard Libraries : math.h, stddef.h, stdint.h, stdio.h, stdlib.h, string.h\n");
    printf(" 8. ACPI Support.\n");
    printf(" 9. APIC Support.\n");
    printf(" 10. RTC Support.\n");
    printf(" 11. SMP Support.\n");
    printf(" 12. PIC Support.\n");
    printf(" 13. AHCI.\n");
    printf(" 14. PIT Timer Support.\n");
    printf(" 15. APIC Timer Support.\n");
    printf(" 14. HPET Timer Support.\n");
    printf(" 14. RTC Timer Support.\n");
    printf(" 14. TSC Timer Support.\n");
    printf("\n");
}


uint64_t get_current_timestamp(){
    uint64_t time = 0;
    asm volatile("rdtsc" : "=A"(time));
    return time;
}

void print_current_timestamp(){
    uint64_t time = get_current_timestamp();
    printf("Current Timestamp: %d\n", time);
}


void print_meminfo() {
    printf("Memory Information:\n");
    printf("Total Physical Memory: %d MB\n", TOTAL_PHYS_MEMORY / (1024 * 1024));
    printf("Usable Physical Memory: %d MB\n", USABLE_LENGTH_PHYS_MEM / (1024 * 1024));
    printf("Kernel Virtual Base Address: %x\n", KERNEL_VIR_BASE);
    printf("Kernel Physical Base Address: %x\n", KERNEL_PHYS_BASE);
    printf("HHDM Offset: %x\n", HHDM_OFFSET);
    printf("Paging Mode: %s\n", (paging_mode == 0) ? "4-Level" : "5-Level");
}

void print_sys_info(){
    printf("System Information:\n");

    // Printing Firmware information
    get_firmware_info();
    print_firmware_info();

    // Printing Limine bootloader information
    get_bootloader_info();
    print_bootloader_info();

    // printing CPU information
    printf(" [*] Total CPU Core: %d\n", getLogicalProcessorCount());
    print_cpu_vendor();
    print_cpu_brand();
    print_cpu_base_frequency();
}


void help(){
    printf("Available commands:\n");
    printf("1. help : Print this help message.\n");
    printf("2. clear : Clear the screen.\n");
    printf("3. reboot : Reboot the system.\n");
    printf("4. poweroff : Power off the system.\n");
    printf("5. calc : Start calculator.\n");    
    printf("6. sl : Start locomotive animation.\n");
    printf("7. call print : Call kernel function.\n");
    printf("8. features : Print OS features.\n");
    printf("9. time : Print current timestamp.\n");
    printf("10. meminfo : Print memory information.\n");
    printf("11. pwd : Print current process ID.\n");
    printf("12. ps : Print process list.\n");
    printf("13. pkill <PID> : Kill process with given PID.\n");
    printf("14. sysinfo : Print system information.\n");
    printf("15. exit : Exit the shell.\n");
    printf("16. pwd : Print current working directory.\n");
    printf("17. touch <filename> : Create a file.\n");
    printf("18. rm <filename> : Remove a file.\n");
    printf("19. mkdir <dirname> : Create a directory.\n");
    printf("20. rmdir <dirname> : Remove a directory.\n");
    printf("21. cd <dirname> : Change directory.\n");
    printf("22. tree : Print directory tree.\n");
}