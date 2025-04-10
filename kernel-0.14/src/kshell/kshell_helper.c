/*
This file is part of the KSH shell.
Copyright (C) 2023 KeblaOS Project
Build Date: 2025-04-05
*/

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"
#include "../process/process.h"
#include "../process/thread.h"

#include "kshell_helper.h"

extern 

void print_os_features(){
    printf("\n");
    printf("KeblaOS-0.12\n");
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





