/*
    CPU details
    Symmetric Multi Processing(SMP)

    https://github.com/limine-bootloader/limine/blob/v7.x/PROTOCOL.md
    https://wiki.osdev.org/CPUID
    https://wiki.osdev.org/CPU_Registers_x86
*/



#include "../lib/stdio.h"

#include "../x86_64/gdt/gdt.h"
#include "../x86_64/gdt/multi_core_gdt_tss.h"

#include "../x86_64/interrupt/interrupt.h"
#include "../x86_64/interrupt/multi_core_interrupt.h"
#include "../x86_64/interrupt/pic.h"
#include "../x86_64/interrupt/apic.h"
#include "../x86_64/interrupt/ioapic.h"
#include "../syscall/int_syscall_manager.h"

#include "../driver/keyboard/keyboard.h"

#include "../x86_64/timer/tsc.h"
#include "../x86_64/timer/apic_timer.h"

#include "../memory/kmalloc.h"
#include "../memory/paging.h"
#include "../memory/pmm.h"

#include "../util/util.h"
#include "../acpi/acpi.h"
#include "../acpi/descriptor_table/madt.h"

#include "../kshell/kshell.h"
#include "../cpu/cpuid.h"

#include "cpu.h"

cpu_data_t cpu_datas[MAX_CPUS];  // Array indexed by CPU ID (APIC ID)

extern struct limine_smp_response *smp_response;

extern madt_t *madt;


void start_bootstrap_cpu_core() {

    asm volatile("cli"); // Disable interrupts

    printf("\n[Info] Starting bootstrap CPU core...\n");

    if (smp_response == NULL) {
        printf("[Error] SMP response is null!\n");
        return;
    }

    disable_pic();
    init_acpi();
    parse_madt(madt);       // Parse MADT to get the LAPIC ID and other information
    enable_ioapic_mode();   // Enable IOAPIC mode

    init_pmm();             // Initialize Physical Memory Manager
    init_paging();          // Initialize paging for the bootstrap core

    init_tsc();             // Initialize TSC for the bootstrap core

    gdt_tss_init();         // Initialize GDT and TSS for the bootstrap core

    // Enable the Interrupts
    int_init();
    init_apic_interrupt();
    int_syscall_init();

    uint32_t bsp_lapic_id = smp_response->bsp_lapic_id;
    // Delivery Mode | Interrupt Mask | Trigger Mode | Polarity | Remote IRR | Delivery Status | Vector
    uint32_t bsp_flags = (0 << 8) | (0 << 13) | (0 << 15);

    // Route Hardware IRQs to the bootstrap core no need to route them to other cores
    ioapic_route_hardware_irq(bsp_lapic_id, bsp_flags);    // Route hardware IRQs to the bootstrap core
    ioapic_route_syscall_irq(bsp_lapic_id, bsp_flags);     // Route syscall IRQs to the bootstrap core

    asm volatile("sti"); // Enable interrupts

    init_apic_timer(10000);
    initKeyboard();

    // if(has_fpu()){
    //     enable_fpu_and_sse();
    //     printf(" [-] FPU and SSE enabled for BSP core %d\n", bsp_lapic_id);
    // }
    
    // Setting up the CPU data structure for the bootstrap core
    cpu_datas[bsp_lapic_id].lapic_id = bsp_lapic_id; // Set the LAPIC ID for the bootstrap core
    cpu_datas[bsp_lapic_id].smp_info = smp_response->cpus[bsp_lapic_id]; // Set the SMP info for the bootstrap core 
    cpu_datas[bsp_lapic_id].is_online = 1; // Mark the bootstrap core as online
    // cpu_datas[bsp_lapic_id].cpu_stack = read_rsp(); // Set the stack pointer for the bootstrap core

    printf("[Info] Bootstrap CPU %d initialized...\n\n", bsp_lapic_id);

    asm volatile("sti");
}


void target_cpu_task(struct limine_smp_info *smp_info) {

    if(smp_info == NULL){
        printf("[Error] SMP INFO is null!\n");
        return;
    }

    uint32_t core_id = smp_info->lapic_id;

    cpu_datas[core_id].lapic_id = core_id;
    cpu_datas[core_id].smp_info = smp_info;

    disable_pic();

    // setting cr3 for this core
    init_core_paging(core_id);
    
    // Initialize the stack for this core
    uint64_t cpu_stack = (uint64_t)kmalloc_a(STACK_SIZE, 1); // Allocate stack for this core
    if(cpu_stack == 0) {
        printf("[Error] Failed to allocate stack for CPU %d\n", core_id);
        return;
    }
    uint64_t cpu_stack_top = cpu_stack + STACK_SIZE; // Set the stack pointer to the top of the allocated stack
    cpu_datas[core_id].cpu_stack = cpu_stack_top;    // Set the stack pointer to the top of the allocated stack
    set_rsp(cpu_stack_top);                          // Set the stack pointer for this core

    // Initialize GDT and TSS for this core
    init_gdt_tss_in_cpu(core_id);

    // Initialize interrupts for this core
    init_ap_core_interrupt(core_id);

    // Initialize APIC for this 
    init_apic_interrupt();

    // if(has_fpu()){
    //     enable_fpu_and_sse();
    // }

    init_apic_timer(100);

    cpu_datas[core_id].is_online = 1; // Mark this core as online
    

    printf(" [-] CPU %d (LAPIC ID: %x) is online\n", core_id, core_id);

    while(true){
        if(core_id == 0 && cpu_datas[core_id].is_online == 1){
            // This is the bootstrap core (BSP)
            printf(" Hello from CPU %d (BSP)\n", core_id);
            for(volatile int i = 0; i < 2000000; i++); // Wait for a while

        } else if (core_id == 1 && cpu_datas[core_id].is_online == 1){
            printf(" Hello from CPU %d (AP)\n", core_id);
            for(volatile int i = 0; i < 1900000; i++); // Wait for a while

        } else if (core_id == 2 && cpu_datas[core_id].is_online == 1){
            printf(" Hello from CPU %d (AP)\n", core_id);
            for(volatile int i = 0; i < 1800000; i++); // Wait for a while

        } else if (core_id == 3 && cpu_datas[core_id].is_online == 1){
            printf(" Hello from CPU %d (AP)\n", core_id);
            for(volatile int i = 0; i < 1700000; i++); // Wait for a while
        } else{
            continue;
        }
    }

    asm volatile("sti"); // Enable interrupts

    // Halt the AP as we have nothing else to do currently
    for (;;) {
        asm volatile("hlt");
    }
}

void start_ap_cpu_cores() {

    if (smp_response == NULL) {
        printf("[Error] SMP response is null!\n");
        return;
    }

    uint32_t start_id = 1;
    uint32_t end_id = smp_response->cpu_count; // Get the number of CPUs from the SMP response
    
    for (uint32_t core = start_id; core < end_id; core++) {

        struct limine_smp_info *smp_info = smp_response->cpus[core];

        // passing sm_info as argument which will accept as input by target_cpu_task
        smp_info->extra_argument = (uint64_t)smp_info;

        // Set function to execute on the AP
        smp_info->goto_address = (limine_goto_address)target_cpu_task;

        for(volatile int i = 0; i < 1000000; i++); // Wait for a while
    }
}



// Initializing all CPU cores
void init_all_cpu_cores() {

    // Initialize the bootstrap core
    start_bootstrap_cpu_core();

    // Initialize the application cores
    start_ap_cpu_cores();

    // Wait for all cores to be online
    while (1) {
        int all_online = 1;
        for (size_t i = 0; i < smp_response->cpu_count; i++) {
            if (!cpu_datas[i].is_online) {
                all_online = 0;
                break;
            }
            printf(" [-] CPU %d is online\n", i);
        }
        if (all_online) {
            break;
        }
    }
    asm volatile("sti");

    printf("[Info] All CPU cores initialized and online.\n");
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



#define MSR_EFER 0xC0000080

static inline uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile (
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );
    return ((uint64_t)high << 32) | low;
}

void check_long_mode() {
    uint64_t efer = read_msr(MSR_EFER);

    if (efer & (1 << 10)) {
        printf("Long mode is active (LMA = 1)\n");
    } else {
        printf("Long mode is NOT active\n");
    }
}

static inline uint64_t read_cr0(void) {
    uint64_t value;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(value));
    return value;
}

static inline uint64_t read_cr4(void) {
    uint64_t value;
    __asm__ volatile ("mov %%cr4, %0" : "=r"(value));
    return value;
}

void check_paging_enabled() {
    uint64_t cr0 = read_cr0();
    uint64_t cr4 = read_cr4();
    uint64_t efer = read_msr(MSR_EFER);

    bool pg = cr0 & (1 << 31); // CR0.PG
    bool pae = cr4 & (1 << 5); // CR4.PAE
    bool lme = efer & (1 << 8); // EFER.LME
    bool lma = efer & (1 << 10); // EFER.LMA

    if (pg)
        printf("Paging is ENABLED (CR0.PG = 1)\n");
    else
        printf("Paging is DISABLED (CR0.PG = 0)\n");

    if (pae)
        printf("PAE is ENABLED (CR4.PAE = 1)\n");

    if (lma)
        printf("Long mode is ACTIVE (EFER.LMA = 1)\n");
}


uint16_t get_cs() {
    uint16_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));

    printf("[Info] CS: %x\n", cs);
    return cs;
}

static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(leaf));
}

bool cpu_supports_long_mode() {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    return edx & (1 << 29);  // LM bit
}

