/*
This file will enable GDT and TSS for every CPU
*/
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../memory/kmalloc.h"

#include "multi_core_gdt_tss.h"

#define MAX_CPUS 256
#define STACK_SIZE 0x4000   // 16 KB


extern void gdt_flush(gdtr_t *gdtr_instance);
extern void tss_flush(uint16_t selector);


cpu_data_t cpu_data[MAX_CPUS];  // Array indexed by CPU ID (APIC ID)

// This function set data of each cpu_data from cpu_data array
void set_cpu_data(size_t cpu_id){
    // Getting cpu_data pointer from cpu_id
    cpu_data_t *cpu_data = &cpu_data[cpu_id];

    uint64_t stack_top = kmalloc_a(STACK_SIZE, true) + STACK_SIZE;
    if(stack_top <= STACK_SIZE) {
        printf("Failed to allocate kernel stack for CPU %d.\n", cpu_id);
        return;
    }

    cpu_data->kernel_stack = stack_top;
}


// Setup GDT entries for each core
void gdt_setup(gdt_entry_t gdt_entries[], uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt_entries[idx].limit_low = limit & 0xFFFF;
    gdt_entries[idx].base_low = base & 0xFFFF;
    gdt_entries[idx].base_middle = (base >> 16) & 0xFF;
    gdt_entries[idx].access = access;
    gdt_entries[idx].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
    gdt_entries[idx].base_high = (base >> 24) & 0xFF;
}

// Setup TSS etry for each core
void tss_setup(gdt_entry_t gdt_entries[], uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    // Populate the lowe 64-bits of the system segment descriptor
    gdt_setup(gdt_entries, idx, base, limit, access, granularity);

    // Populate the upper 64-bits of the system segment descriptor
    gdt_entries[idx + 1].base_upper = (base >> 32) & 0xFFFFFFFF;
    gdt_entries[idx + 1].reserved = 0;
}

// Initialize GDT and TSS for a CPU
void init_gdt_tss_in_cpu(size_t cpu_id){
    if(cpu_id >= MAX_CPUS) {
        printf("Invalid core ID %d\n", cpu_id);
        return;
    }

    set_cpu_data(cpu_id);

    // Getting cpu_data pointer from cpu_id
    cpu_data_t *cpu = &cpu_data[cpu_id];

    // Set GDT Entries for this cpu
    gdt_setup(cpu->gdt_entries, 0, 0, 0x0, 0x0, 0x0);      // Null
    gdt_setup(cpu->gdt_entries, 1, 0, 0xFFFF, 0x9A, 0xA0); // Kernel Code Selector 0x08
    gdt_setup(cpu->gdt_entries, 2, 0, 0xFFFF, 0x92, 0xA0); // Kernel Data Selector 0x10
    gdt_setup(cpu->gdt_entries, 3, 0, 0xFFFF, 0xFA, 0xA0); // User Code Selector   0x18
    gdt_setup(cpu->gdt_entries, 4, 0, 0xFFFF, 0xF2, 0xA0); // User Data Selector   0x20

    // Set TSS Entries for this cpu
    memset(&cpu->tss, 0, sizeof(cpu->tss));
    cpu->tss.rsp0 = cpu->kernel_stack;
    cpu->tss.iopb_offset = sizeof(cpu->tss);
    tss_setup(cpu->gdt_entries, 5, (uint64_t)&cpu->tss, sizeof(tss_t), 0x89, 0x0 );

    // Load The above GDT and TSS
    cpu->gdtr.limit = (uint16_t) (sizeof(gdt_entry_t) * 7 - 1); // 16 * 7 - 1 = 111 bytes
    cpu->gdtr.base = (uint64_t) &cpu->gdt_entries;

    gdt_flush(&cpu->gdtr);  // Load GDT
    tss_flush(0x28);        // Selector 0x28 (5th entry in GDT)

    printf("[Info] Initialize GDT & TSS for CPU %d.\n", cpu_id);
}








