/*
    CPU details
    Symmetric Multi Processing(SMP)

    https://github.com/limine-bootloader/limine/blob/v7.x/PROTOCOL.md
    https://wiki.osdev.org/CPUID
    https://wiki.osdev.org/CPU_Registers_x86
*/


#include "../../../limine-8.6.0/limine.h"
#include "../lib/stdio.h"
#include "../x86_64/gdt/gdt.h"
#include "../x86_64/gdt/multi_core_gdt_tss.h"
#include "../x86_64/interrupt/multi_core_interrupt.h"
#include "../x86_64/interrupt/apic.h"
#include "../x86_64/interrupt/ioapic.h"
#include "../driver/keyboard/keyboard.h"
#include "../x86_64/timer/tsc.h"
#include "../x86_64/timer/pit_timer.h"
#include "../x86_64/timer/apic_timer.h"
#include "../memory/kmalloc.h"
#include "../util/util.h"
#include "../acpi/acpi.h"
#include "../acpi/descriptor_table/madt.h"
#include "../memory/paging.h"
#include "../memory/pmm.h"
#include "../syscall/int_syscall_manager.h"

#include "cpu.h"



extern struct limine_smp_response *smp_response;

// Define stack size and storage
#define MAX_CORES 16
#define STACK_SIZE 4096 * 4       // 16 KB per core

uint64_t ap_stacks[MAX_CORES];   // Stores stack for each AP core

extern madt_t *madt;


// Allocate stack for each AP core
void create_ap_stacks(int start_id, int end_id) {
    for (int i = start_id; i < end_id; i++) {
        ap_stacks[i] = (uint64_t) kmalloc_a(STACK_SIZE, 1);
        if (!ap_stacks[i]) {
            printf("[Error] CPU: Failed to allocate AP %d stack!\n", i);
            return;
        }
    }
}


void start_bootstrap_cpu_core() {
    uint32_t bsp_lapic_id = smp_response->bsp_lapic_id;
    asm volatile("cli");
    init_acpi();
    parse_madt(madt);

    gdt_tss_init();
    init_bootstrap_interrupt(bsp_lapic_id);
    init_tsc();
    init_apic_interrupt();

    init_paging();
   
    // Route IRQs to the bootstrap core
    uint32_t bsp_flags = (0 << 8) | (0 << 13) | (0 << 15);
    
    ioapic_route_irq(0, bsp_lapic_id, 32, bsp_flags);      // Route IRQ 1 to current LAPIC ID with vector 33
    ioapic_route_irq(1, bsp_lapic_id, 33, bsp_flags);      // Route IRQ 1 to current LAPIC ID with vector 33
    ioapic_route_irq(2, bsp_lapic_id, 34, bsp_flags);      // Route IRQ 2 to current LAPIC ID with vector 34
    ioapic_route_irq(3, bsp_lapic_id, 35, bsp_flags);      // Route IRQ 3 to current LAPIC ID with vector 35
    ioapic_route_irq(4, bsp_lapic_id, 36, bsp_flags);      // Route IRQ 4 to current LAPIC ID with vector 36
    ioapic_route_irq(5, bsp_lapic_id, 37, bsp_flags);      // Route IRQ 5 to current LAPIC ID with vector 37
    ioapic_route_irq(6, bsp_lapic_id, 38, bsp_flags);      // Route IRQ 6 to current LAPIC ID with vector 38
    ioapic_route_irq(7, bsp_lapic_id, 39, bsp_flags);      // Route IRQ 7 to current LAPIC ID with vector 39
    ioapic_route_irq(8, bsp_lapic_id, 40, bsp_flags);      // Route IRQ 8 to current LAPIC ID with vector 40
    ioapic_route_irq(9, bsp_lapic_id, 41, bsp_flags);      // Route IRQ 9 to current LAPIC ID with vector 41
    ioapic_route_irq(10, bsp_lapic_id, 42, bsp_flags);     // Route IRQ 10 to current LAPIC ID with vector 42
    ioapic_route_irq(11, bsp_lapic_id, 43, bsp_flags);     // Route IRQ 11 to current LAPIC ID with vector 43
    ioapic_route_irq(12, bsp_lapic_id, 44, bsp_flags);     // Route IRQ 12 to current LAPIC ID with vector 44
    ioapic_route_irq(13, bsp_lapic_id, 45, bsp_flags);     // Route IRQ 13 to current LAPIC ID with vector 45
    ioapic_route_irq(14, bsp_lapic_id, 46, bsp_flags);     // Route IRQ 14 to current LAPIC ID with vector 46
    ioapic_route_irq(15, bsp_lapic_id, 47, bsp_flags);     // Route IRQ 15 to current LAPIC ID with vector 47
    ioapic_route_irq(16, bsp_lapic_id, 48, bsp_flags);     // Route IRQ 16 to current LAPIC ID with vector 48
    ioapic_route_irq(17, bsp_lapic_id, 49, bsp_flags);     // Route IRQ 17 to current LAPIC ID with vector 49
    ioapic_route_irq(18, bsp_lapic_id, 50, bsp_flags);     // Route IRQ 18 to current LAPIC ID with vector 50

    ioapic_route_irq(140, bsp_lapic_id, 172, bsp_flags);   // Route IRQ 140 to current LAPIC ID with vector 172
    ioapic_route_irq(141, bsp_lapic_id, 173, bsp_flags);   // Route IRQ 141 to current LAPIC ID with vector 173
    ioapic_route_irq(142, bsp_lapic_id, 174, bsp_flags);   // Route IRQ 142 to current LAPIC ID with vector 174

    init_apic_timer(100);
    initKeyboard();
    int_syscall_init();

    asm volatile("sti");
    printf("[Info] Bootstrap CPU initialized...\n\n");
}


void target_cpu_task(struct limine_smp_info *smp_info) {

    if(smp_info == NULL){
        printf("[Error] SMP INFO is null!\n");
        return;
    }

    uint32_t core_id = smp_info->lapic_id;
    init_acpi();
    parse_madt(madt);
    init_tsc();

    // The limine automatically allocates a stack for each CPU core
    uint64_t stack_top = ap_stacks[core_id] + STACK_SIZE;

    asm volatile("cli");

    // Set the stack pointer
    set_rsp(stack_top);

    // Initialize GDT and TSS for this core
    init_gdt_tss_in_cpu(core_id);

    // Initialize interrupts for this core
    init_core_interrupt(core_id);

    // Initialize APIC for this 
    init_apic_interrupt();

    init_core_paging(core_id);

    // Enable interrupts for this core
    asm volatile("sti");

    printf(" [-] CPU %d (LAPIC ID: %x) is online\n", core_id, core_id);

    // Testing system call from AP core
    // asm volatile("int $0x0"); // System call interrupt

    // Halt the AP as we have nothing else to do currently
    for (;;) {
        asm volatile("hlt");
    }

}

void start_secondary_cpu_cores(int start_id, int end_id) {

    if (smp_response == NULL) {
        printf("[Error] SMP response is null!\n");
        return;
    }
    
    create_ap_stacks(start_id, end_id);    // Creating stacks for all AP cores

    for (int core = start_id; core < end_id; core++) {
        struct limine_smp_info *smp_info = smp_response->cpus[core];

        // passing sm_info as argument which will accept as input by target_cpu_task
        smp_info->extra_argument = (uint64_t)smp_info;

        // Set function to execute on the AP
        smp_info->goto_address = (limine_goto_address)target_cpu_task;

        for(volatile int i = 0; i < 1000000; i++); // Wait for a while

        printf(" [-] CPU %d Started\n", core);
    }
    printf("[Info] CPU: Successfully started all cores\n");
}


// Initializing all CPU cores
void init_all_cpu_cores() {
    // Creating Stacks for all cpus
    create_ap_stacks(0, smp_response->cpu_count);

    // Initialize the bootstrap core
    start_bootstrap_cpu_core();

    // Initialize the application cores
    start_secondary_cpu_cores(1, smp_response->cpu_count);
}


void switch_to_core(uint32_t target_lapic_id) {
    if (smp_response == NULL) return;

    for (size_t i = 0; i < smp_response->cpu_count; i++) {
        struct limine_smp_info *smp_info = smp_response->cpus[i];
        if (smp_info->lapic_id == target_lapic_id) {
            // Send IPI to the target core
            lapic_send_ipi(target_lapic_id, 0x40); // 0x20 is the vector
            break;
        }
    }

    printf("[Info] Successfully Switched  into CPU %d.\n", target_lapic_id);
}







