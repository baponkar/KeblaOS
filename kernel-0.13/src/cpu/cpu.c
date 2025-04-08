/*
    CPU details
    Symmetric Multi Processing(SMP)

    https://github.com/limine-bootloader/limine/blob/v9.x/PROTOCOL.md
    https://wiki.osdev.org/CPUID
*/

#include <cpuid.h>

#include "../../../limine-8.6.0/limine.h"
#include "../lib/stdio.h"

#include "../x86_64/gdt/gdt.h"

#include "../x86_64/interrupt/interrupt.h"
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

#include "cpu.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".requests")))
static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 3
};


// Define stack size and storage
#define MAX_CORES 16
#define STACK_SIZE 4096 * 4  // 16 KB per core
static uint8_t *ap_stacks[MAX_CORES];   // Stores stack for each AP core


// Getting CPU information using Limine Bootloader
struct limine_smp_info ** get_cpus(){
    if(smp_request.response != NULL)  return smp_request.response->cpus;
    return NULL;
}

limine_goto_address get_goto_address(int core_id){
    if(smp_request.response != NULL)  return smp_request.response->cpus[core_id]->goto_address;
    return NULL;
}

uint64_t get_extra_argument(int core_id){
    if(smp_request.response != NULL)  return smp_request.response->cpus[core_id]->extra_argument;
    return 0;
}


uint64_t get_revision(){
    if(smp_request.response != NULL)  return smp_request.response->revision;
    return 0;
}

uint32_t get_flags(){
    if(smp_request.response != NULL)  return smp_request.response->flags;
    return 0;
}

int get_cpu_count(){
    if(smp_request.response != NULL)  return (int) smp_request.response->cpu_count;
    return 1; // Defult one processor count
}

int get_bsp_lapic_id(){
    if(smp_request.response != NULL)  return (int) smp_request.response->bsp_lapic_id;
    return 0; // Defult one bootstrap processor count
}

uint32_t get_lapic_id_by_limine(int core_id){
    if(smp_request.response != NULL)  return smp_request.response->cpus[core_id]->lapic_id;
    return 0; // Defult one processor count
}

// Allocate stack for each AP core
void set_ap_stacks(int start_id, int end_id) {
    for (int i = start_id; i < end_id; i++) {
        ap_stacks[i] = (uint8_t *) kmalloc_a(STACK_SIZE, 1);
        if (!ap_stacks[i]) {
            printf("Failed to allocate AP stack\n");
        }
    }
}


void target_cpu_task(struct limine_smp_info *smp_info) {
    int core_id = (int) smp_info->lapic_id;

    uint8_t *stack_top = ap_stacks[core_id] + STACK_SIZE;

    // Set the stack pointer
    set_rsp((uint64_t) stack_top);

    // init_acpi(); // Alreadt ACPI Initialized in bootstrap core
    
    // Initialize GDT and TSS for this core
    init_core_gdt_tss(core_id);

    // Initialize interrupts for this core
    init_core_interrupt(core_id);

    // Do not need to initialize TSC for application core
    // init_tsc();

    // Initialize APIC for this 
    init_apic_interrupt();

    // Configure APIC timer for this core
    // init_apic_timer(150); // 150 ms interval
    switch (core_id)
    {
    case 1:
        init_apic_timer(120); // 100 ms interval
        break;
    case 2:
        init_apic_timer(130); // 130 ms interval
        break;
    case 3:
        init_apic_timer(140); // 140 ms interval
        break;
    default:
        init_apic_timer(100); // 100 ms interval
        break;
    }

    printf("[Info] CPU %d: APIC Timer initialized!\n", core_id);

    // Route IRQs to the current core
    uint32_t lapic_flags = (0 << 8) | (0 << 13) | (0 << 15);


    if(core_id == 1){
        // ioapic_route_irq(1, get_lapic_id(), 33, lapic_flags);      // Route IRQ 1 to current LAPIC ID with vector 33
        ioapic_route_irq(2, get_lapic_id(), 34, lapic_flags);      // Route IRQ 2 to current LAPIC ID with vector 34
        ioapic_route_irq(3, get_lapic_id(), 35, lapic_flags);      // Route IRQ 3 to current LAPIC ID with vector 35
        ioapic_route_irq(4, get_lapic_id(), 36, lapic_flags);      // Route IRQ 4 to current LAPIC ID with vector 36
        ioapic_route_irq(5, get_lapic_id(), 37, lapic_flags);      // Route IRQ 5 to current LAPIC ID with vector 37
        ioapic_route_irq(6, get_lapic_id(), 38, lapic_flags);      // Route IRQ 6 to current LAPIC ID with vector 38
    }else if(core_id == 2){
        ioapic_route_irq(7, get_lapic_id(), 39, lapic_flags);      // Route IRQ 7 to current LAPIC ID with vector 39
        ioapic_route_irq(8, get_lapic_id(), 40, lapic_flags);      // Route IRQ 8 to current LAPIC ID with vector 40
        ioapic_route_irq(9, get_lapic_id(), 41, lapic_flags);      // Route IRQ 9 to current LAPIC ID with vector 41
        ioapic_route_irq(10, get_lapic_id(), 42, lapic_flags);     // Route IRQ 10 to current LAPIC ID with vector 42
        ioapic_route_irq(11, get_lapic_id(), 43, lapic_flags);     // Route IRQ 11 to current LAPIC ID with vector 43
        ioapic_route_irq(12, get_lapic_id(), 44, lapic_flags);     // Route IRQ 12 to current LAPIC ID with vector 44
    }else {
        ioapic_route_irq(13, get_lapic_id(), 45, lapic_flags);     // Route IRQ 13 to current LAPIC ID with vector 45
        ioapic_route_irq(14, get_lapic_id(), 46, lapic_flags);     // Route IRQ 14 to current LAPIC ID with vector 46
        ioapic_route_irq(15, get_lapic_id(), 47, lapic_flags);     // Route IRQ 15 to current LAPIC ID with vector 47
        ioapic_route_irq(16, get_lapic_id(), 48, lapic_flags);     // Route IRQ 16 to current LAPIC ID with vector 48
        ioapic_route_irq(17, get_lapic_id(), 49, lapic_flags);     // Route IRQ 17 to current LAPIC ID with vector 49
        ioapic_route_irq(18, get_lapic_id(), 50, lapic_flags);     // Route IRQ 18 to current LAPIC ID with vector 50

        initKeyboard();
    }
    

    // Enable interrupts for this core
    asm volatile("sti");

    // Enter ap core loop or wait for tasks
    while (1) {
        asm volatile("hlt");
    }
}


void switch_to_core(uint32_t target_lapic_id) {
    if (smp_request.response == NULL) return;

    for (size_t i = 0; i < smp_request.response->cpu_count; i++) {
        struct limine_smp_info *smp_info = smp_request.response->cpus[i];
        if (smp_info->lapic_id == target_lapic_id) {
            // Send IPI to the target core
            lapic_send_ipi(target_lapic_id, 0x40); // 0x20 is the vector
            break;
        }
    }
    printf("[Info] Successfully Switched  into CPU %d.\n", target_lapic_id);
}



void start_bootstrap_cpu_core() {
    uint32_t bsp_lapic_id = get_bsp_lapic_id();
    asm volatile("cli");
    init_acpi();
    parse_madt(madt_addr);

    init_bootstrap_gdt_tss(bsp_lapic_id);
    init_bootstrap_interrupt(bsp_lapic_id);
    init_tsc();
    init_apic_interrupt();
    
    init_apic_timer(100);

    // Route IRQs to the bootstrap core
    uint32_t bsp_flags = (0 << 8) | (0 << 13) | (0 << 15);
    // ioapic_route_irq(0, bsp_lapic_id, 32, bsp_flags);      // Route IRQ 1 to current LAPIC ID with vector 33
    ioapic_route_irq(1, bsp_lapic_id, 33, bsp_flags);      // Route IRQ 1 to current LAPIC ID with vector 33
    // ioapic_route_irq(2, bsp_lapic_id, 34, bsp_flags);      // Route IRQ 2 to current LAPIC ID with vector 34
    // ioapic_route_irq(3, bsp_lapic_id, 35, bsp_flags);      // Route IRQ 3 to current LAPIC ID with vector 35
    // ioapic_route_irq(4, bsp_lapic_id, 36, bsp_flags);      // Route IRQ 4 to current LAPIC ID with vector 36
    // ioapic_route_irq(5, bsp_lapic_id, 37, bsp_flags);      // Route IRQ 5 to current LAPIC ID with vector 37
    // ioapic_route_irq(6, bsp_lapic_id, 38, bsp_flags);      // Route IRQ 6 to current LAPIC ID with vector 38
    // ioapic_route_irq(7, bsp_lapic_id, 39, bsp_flags);      // Route IRQ 7 to current LAPIC ID with vector 39
    // ioapic_route_irq(8, bsp_lapic_id, 40, bsp_flags);      // Route IRQ 8 to current LAPIC ID with vector 40
    // ioapic_route_irq(9, bsp_lapic_id, 41, bsp_flags);      // Route IRQ 9 to current LAPIC ID with vector 41
    // ioapic_route_irq(10, bsp_lapic_id, 42, bsp_flags);     // Route IRQ 10 to current LAPIC ID with vector 42
    // ioapic_route_irq(11, bsp_lapic_id, 43, bsp_flags);     // Route IRQ 11 to current LAPIC ID with vector 43
    // ioapic_route_irq(12, bsp_lapic_id, 44, bsp_flags);     // Route IRQ 12 to current LAPIC ID with vector 44
    // ioapic_route_irq(13, bsp_lapic_id, 45, bsp_flags);     // Route IRQ 13 to current LAPIC ID with vector 45
    // ioapic_route_irq(14, bsp_lapic_id, 46, bsp_flags);     // Route IRQ 14 to current LAPIC ID with vector 46
    // ioapic_route_irq(15, bsp_lapic_id, 47, bsp_flags);     // Route IRQ 15 to current LAPIC ID with vector 47
    // ioapic_route_irq(16, bsp_lapic_id, 48, bsp_flags);     // Route IRQ 16 to current LAPIC ID with vector 48
    // ioapic_route_irq(17, bsp_lapic_id, 49, bsp_flags);     // Route IRQ 17 to current LAPIC ID with vector 49
    // ioapic_route_irq(18, bsp_lapic_id, 50, bsp_flags);     // Route IRQ 18 to current LAPIC ID with vector 50

    // init_pit_timer();
    initKeyboard();

    asm volatile("sti");
    printf("[Info] Bootstrap CPU initialized...\n");
}



void start_secondary_cpu_cores(int start_id, int end_id) {
    for (int core = start_id; core < smp_request.response->cpu_count; core++) {
        struct limine_smp_info *smp_info = smp_request.response->cpus[core];

        // passing sm_info as argument which will accept as input by target_cpu_task
        smp_info->extra_argument = (uint64_t)smp_info;

        // Set function to execute on the AP
        smp_info->goto_address = (limine_goto_address) target_cpu_task;

        // Short delay to let APs start (only for debugging)
        for (volatile int i = 0; i < 1000000; i++);
    }
}


// Initializing all CPU cores
void init_all_cpu_cores() {
    if (smp_request.response == NULL) return;

    // Initialize the bootstrap core
    start_bootstrap_cpu_core();

    // Initialize the application cores
    start_secondary_cpu_cores(1, smp_request.response->cpu_count);
}



void get_cpu_info(){
    if(!smp_request.response) printf("[Info] No CPU info found!\n");

    uint64_t revision = smp_request.response->revision;
    uint32_t flags = smp_request.response->flags;
    uint32_t bsp_lapic_id = smp_request.response->bsp_lapic_id;
    uint64_t cpu_count = smp_request.response->cpu_count;

    struct limine_smp_info ** cpus = smp_request.response->cpus;

    for(size_t i=0; i<(size_t) cpu_count; i++){
        uint32_t processor_id = cpus[i]->processor_id;

        uint32_t lapic_id = cpus[i]->lapic_id;
        uint64_t reserved = cpus[i]->reserved;

        limine_goto_address goto_address = cpus[i]->goto_address;
        uint64_t extra_argument = cpus[i]->extra_argument;

        // printf("LAPIC ID: %d\n", lapic_id);
    }
}

// Debugging
void print_cpu_info(){
    if(!smp_request.response) printf("[Info] No CPU info found!\n");
    
    uint64_t revision = smp_request.response->revision;
    uint32_t flags = smp_request.response->flags;
    uint32_t bsp_lapic_id = smp_request.response->bsp_lapic_id;
    uint64_t cpu_count = smp_request.response->cpu_count;
    struct limine_smp_info ** cpus = smp_request.response->cpus;

    printf("[Info] CPU info : \nRevision : %d, Flags : %d, BSP LAPIC ID : %d, CPU count : %d\n", revision, flags, bsp_lapic_id, cpu_count);

    for(size_t i=0; i<(size_t) cpu_count; i++){
        uint32_t processor_id = cpus[i]->processor_id;
        uint32_t lapic_id = cpus[i]->lapic_id;
        uint64_t reserved = cpus[i]->reserved;
        limine_goto_address goto_address = cpus[i]->goto_address;
        uint64_t extra_argument = cpus[i]->extra_argument;

        printf("[Info] Processor ID : %d, LAPIC ID : %d, Reserved : %x, Extra argument : %x\n", processor_id,lapic_id, reserved, extra_argument );
    }
}







