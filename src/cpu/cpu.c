/*
    CPU details
    Symmetric Multi Processing(SMP)

    https://github.com/limine-bootloader/limine/blob/v9.x/PROTOCOL.md
    https://wiki.osdev.org/CPUID
*/

#include <cpuid.h>

#include "../limine/limine.h"
#include "../lib/stdio.h"

#include "../x86_64/gdt/gdt.h"

#include "../x86_64/interrupt/interrupt.h"
#include "../x86_64/interrupt/apic.h"
#include "../x86_64/interrupt/ioapic.h"

#include "../driver/keyboard/keyboard.h"

#include "../x86_64/timer/tsc.h"
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


void get_cpu_info(){
    if(!smp_request.response) printf("No CPU info found!\n");

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




void target_cpu_task(struct limine_smp_info *smp_info) {
    int core_id = (int) smp_info->lapic_id;

    uint8_t *stack_top = ap_stacks[core_id] + STACK_SIZE;

    // Set the stack pointer
    set_rsp((uint64_t) stack_top);

    init_acpi();
    
    // Initialize GDT and TSS for this core
    init_core_gdt_tss(core_id);

    // Initialize interrupts for this core
    init_core_interrupt(core_id);

    // Do not need to initialize TSC for application core
    // init_tsc();

    // Initialize APIC for this 
    init_apic_interrupt();

    // Configure APIC timer for this core
    init_apic_timer(150); // 150 ms interval

    // void ioapic_route_irq(uint8_t irq, uint8_t apic_id, uint8_t vector, uint32_t flags);
    // ioapic_route_irq(0, get_bsp_lapic_id(), 32, (0 << 8) | (1 << 15)); // Route IRQ 0 to current LAPIC ID with vector 32
    ioapic_route_irq(1, 0, 0x21, (0 << 8) | (1 << 15)); // Route IRQ 1 to LAPIC ID 0 with vector 0x21

    initKeyboard();

    printf("CPU %d: APIC Timer initialized!\n", core_id);

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
    printf("Successfully Switched  into CPU %d\n", target_lapic_id);
}



void start_bootstrap_cpu_core() {
    printf("Initializing Bootstrap CPU...\n");
    uint32_t bsp_lapic_id = smp_request.response->bsp_lapic_id;
    asm volatile("cli");
    init_acpi();
    parse_madt(madt_addr);

    init_bootstrap_gdt_tss(bsp_lapic_id);
    init_bootstrap_interrupt(bsp_lapic_id);

    init_tsc();

    init_apic_interrupt();
    
    init_apic_timer(100);

    // ioapic_route_irq(0, get_bsp_lapic_id(), 32, (0 << 8) | (1 << 15));  // Route IRQ 0 to current LAPIC ID with vector 32
    ioapic_route_irq(1, get_lapic_id(), 33, (0 << 8) | (1 << 15));      // Route IRQ 1 to current LAPIC ID with vector 33
    initKeyboard();

    asm volatile("sti");
    printf("Bootstrap CPU initialized...\n\n");
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

        printf("Initializing CPU %d (LAPIC ID %d)...\n\n", core, smp_info->lapic_id);
    }
}


void init_cpu(){
    get_cpu_info();
    print_cpu_info();

    ap_stacks[0] = (uint8_t *) read_rsp();
}


// Debugging
void print_cpu_info(){
    if(!smp_request.response) printf("No CPU info found!\n");
    
    uint64_t revision = smp_request.response->revision;
    uint32_t flags = smp_request.response->flags;
    uint32_t bsp_lapic_id = smp_request.response->bsp_lapic_id;
    uint64_t cpu_count = smp_request.response->cpu_count;
    struct limine_smp_info ** cpus = smp_request.response->cpus;

    printf("CPU info : \nRevision : %d, Flags : %d, BSP LAPIC ID : %d, CPU count : %d\n", revision, flags, bsp_lapic_id, cpu_count);

    for(size_t i=0; i<(size_t) cpu_count; i++){
        uint32_t processor_id = cpus[i]->processor_id;
        uint32_t lapic_id = cpus[i]->lapic_id;
        uint64_t reserved = cpus[i]->reserved;
        limine_goto_address goto_address = cpus[i]->goto_address;
        uint64_t extra_argument = cpus[i]->extra_argument;

        printf("Processor ID : %d, LAPIC ID : %d, Reserved : %x, Extra argument : %x\n", processor_id,lapic_id, reserved, extra_argument );
    }
}







