
/*
    CPU details
    Symmetric Multi Processing(SMP)

    https://github.com/limine-bootloader/limine/blob/v7.x/PROTOCOL.md
    https://wiki.osdev.org/CPUID
    https://wiki.osdev.org/CPU_Registers_x86
*/



#include "../../lib/stdio.h"

#include "smp.h"

#include "../../arch/gdt/gdt.h"
#include "../../arch/gdt/multi_core_gdt_tss.h"

#include "../../arch/interrupt/pic/pic.h"
#include "../../arch/interrupt/pic/pic_interrupt.h"

#include "../../arch/interrupt/apic/apic_interrupt.h"
#include "../../arch/interrupt/apic/apic.h"
#include "../../arch/interrupt/apic/ioapic.h"
#include "../../syscall/int_syscall_manager.h"

#include "../../driver/keyboard/keyboard.h"

#include "../../sys/timer/tsc.h"
#include "../../sys/timer/pit_timer.h"
#include "../../sys/timer/apic_timer.h"

#include "../../memory/detect_memory.h"
#include "../../memory/kmalloc.h"
#include "../../memory/paging.h"
#include "../../memory/pmm.h"

#include "../../util/util.h"
#include "../acpi/acpi.h"
#include "../acpi/descriptor_table/madt.h"

#include "../../kshell/kshell.h"
#include "cpuid.h"

#include "../../arch/interrupt/apic/ipi.h"

#include "../../syscall/syscall_manager.h"

#include "cpu.h"




cpu_data_t cpu_datas[MAX_CPUS];  // Array indexed by CPU ID (APIC ID)

extern struct limine_smp_response *smp_response;

extern madt_t *madt;

// This function initializes the bootstrap CPU core with PIC
void init_bs_cpu_core(){
    print_cpu_brand();
    print_cpu_vendor();
    print_cpu_base_frequency();
    get_set_memory();
    get_smp_info();

    // initially starts pic
    gdt_tss_init();         // Initialize GDT and TSS
    pic_int_init();         // Initialize PIC Interrupts
    init_pmm();             // Initialize Physical Memory Manager
    init_bs_paging();       // Initialize paging for the bootstrap core
    init_pit_timer(100);    // Initialize PIT Timer
    init_tsc();             // Initialize TSC for the bootstrap core

    printf("[Info] CPU %d (Bootstrap) with PIC initialized...\n\n", 0);
}


void start_bootstrap_cpu_core() {

    if (smp_response == NULL) {
        printf("[Error] SMP response is null!\n");
        return;
    }

    uint32_t bsp_lapic_id = smp_response->bsp_lapic_id;

    asm volatile("cli");        // Disable interrupts

    disable_pic();              // Disable PIC interrupts

    disable_pit_timer();        // Disable PIT timer interrupts

    get_set_memory();           // Get memory information and set usable memory map

    init_acpi();                // Initialize ACPI

    parse_madt(madt);           // Parse MADT to get the LAPIC ID and other information

    gdt_tss_init();             // Initialize GDT and TSS for the bootstrap core

    init_apic();                // Initialize the APIC & IOAPIC for the bootstrap core

    bsp_apic_int_init();        // Initialize APIC Interrupts

    init_pmm();                 // Initialize Physical Memory Manager for the bootstrap core

    init_bs_paging();           // Initialize paging for the bootstrap core
    // init_bs_paging_with_new_pml4();


    int_syscall_init();         // Initialize int based system calls for the bootstrap core    
    init_ipi();                 // Initialize IPI for inter-processor communication

    init_syscall(0);            // Initialize system calls for the bootstrap core

    asm volatile("sti");        // Enable interrupts

    enable_fpu_and_sse();       // Enable FPU and SSE for the bootstrap core

    init_apic_timer(100);       // Initialize the APIC timer for the bootstrap core
    initKeyboard();             // Initialize the keyboard driver

    // Setting up the CPU data structure for the bootstrap core
    cpu_datas[bsp_lapic_id].lapic_id = bsp_lapic_id;                        // Set the LAPIC ID for the bootstrap core
    cpu_datas[bsp_lapic_id].smp_info = smp_response->cpus[bsp_lapic_id];    // Set the SMP info for the bootstrap core 
    cpu_datas[bsp_lapic_id].is_online = 1;                                  // Mark the bootstrap core as online
    cpu_datas[bsp_lapic_id].cpu_stack = read_rsp();                         // Set the stack pointer for the bootstrap core

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

    get_set_memory(); // already done in start_bootstrap_cpu_core()


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
    ap_apic_int_init(core_id);
    init_ipi();   
    
    // Initialize Physical Memory Manager
    init_pmm(); // Already done in start_bootstrap_cpu_core()
    init_ap_paging(core_id);

    // Initialize the FPU and SSE for this core
    if(has_fpu()){
        enable_fpu_and_sse();
    }

    cpu_datas[core_id].is_online = 1; // Mark this core as online
    printf(" [-] CPU %d (LAPIC ID: %x) is online\n", core_id, core_id);

    asm volatile("sti"); // Enable interrupts

    // for(size_t core_id = 0; core_id < smp_response->cpu_count; core_id++) {
    //     if(cpu_datas[core_id].is_online == 1){
    //         printf(" [-] CPU %d is online\n", core_id);
    //         tsc_sleep(1000000); // Wait for one second
    //     }

    //     if(core_id == 3 && cpu_datas[core_id].is_online == 1){
    //         printf(" Hello from CPU %d (AP)\n", core_id);
    //         tsc_sleep(1000000); // Wait for one second
    //     }
    // }

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

        tsc_sleep(1000000); // Wait 1s before starting the next core

        printf("[Info] AP CPU core %d started...\n", core);
    }
}



// Initializing all CPU cores
void init_all_cpu_cores() {

    // Initialize the bootstrap core
    start_bootstrap_cpu_core();

    // Initialize the application cores
    // start_ap_cpu_cores();

    // Wait for all cores to be online
    // while (1) {
    //     int all_online = 1;
    //     for (size_t i = 0; i < smp_response->cpu_count; i++) {
    //         if (!cpu_datas[i].is_online) {
    //             all_online = 0;
    //             break;
    //         }
    //         printf(" [-] CPU %d is online\n", i);
    //     }
    //     if (all_online) {
    //         break;
    //     }
    // }
    asm volatile("sti");

    printf("[Info] All CPU cores initialized and online.\n");
}

void switch_to_core(uint32_t target_lapic_id) {
    if (smp_response == NULL) return;

    for (size_t i = 0; i < smp_response->cpu_count; i++) {
        struct limine_smp_info *smp_info = smp_response->cpus[i];
        if (smp_info->lapic_id == target_lapic_id) {
            // Send IPI to the target core
            lapic_send_ipi(target_lapic_id, 50); // 50 is the vector for the IPI
            break;
        }
    }

    printf("[Info] Successfully Switched  into CPU %d.\n", target_lapic_id);
}




