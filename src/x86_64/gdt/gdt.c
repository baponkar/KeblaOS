
/*
    Global Descriptor Table
    Task State Segment

    https://wiki.osdev.org/Global_Descriptor_Table
    https://wiki.osdev.org/GDT_Tutorial
    https://web.archive.org/web/20160326064709/http://jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
    http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial
    https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/04_GDT.md
    https://stackoverflow.com/questions/79260987/why-gdt-is-not-working-in-the-x86-64-system
    https://forum.osdev.org/viewtopic.php?t=57521
    https://stackoverflow.com/questions/37554399/what-is-the-use-of-defining-a-global-descriptor-table

    https://wiki.osdev.org/Task_State_Segment

*/
#include "../../cpu/cpu.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../mmu/kmalloc.h"
#include "../../lib/assert.h"
#include "../interrupt/apic.h"

#include "gdt.h"


#define STACK_SIZE 0x4000  // 16 KB

extern void gdt_flush(gdtr_t *gdtr_instance);
extern void tss_flush(uint16_t selector);

#define MAX_CORES 256

cpu_data_t cpu_data[MAX_CORES];  // Array indexed by CPU ID (APIC ID)
cpu_data_t bootstrap;            // Bootstrap processor

// Initialize GDT and TSS for Bootstrap CPU
void start_bootstrap_gdt_tss() {
    init_gdt_tss(&bootstrap);

    if (!bootstrap.kernel_stack) {
        printf("Error: Stack allocation failed\n");
        return;
    }

    load_gdt_tss(&bootstrap);

    printf("Successfully started GDT and TSS for Bootstrap CPU.\n");
}

int detect_cores(){
    return get_cpu_count();
}

void gdt_setup(gdt_entry_t gdt[], uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[idx].limit_low = limit & 0xFFFF;
    gdt[idx].base_low = base & 0xFFFF;
    gdt[idx].base_middle = (base >> 16) & 0xFF;
    gdt[idx].access = access;
    gdt[idx].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
    gdt[idx].base_high = (base >> 24) & 0xFF;
}

// Setup a system segment descriptor by creating 2 consecutive 64-bit GDT descriptors to make one 128-bit entry
void gdt_setup_sysseg(gdt_entry_t gdt[], uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    // Populate the lowe 64-bits of the system segment descriptor
    gdt_setup(gdt, idx, base, limit, access, granularity);

    // Populate the upper 64-bits of the system segment descriptor
    gdt[idx + 1].base_upper = (base >> 32) & 0xFFFFFFFF;
    gdt[idx + 1].reserved = 0;
}

void init_gdt_tss(cpu_data_t *core) {
    uint64_t stack = kmalloc_a(STACK_SIZE, true);
    core->kernel_stack = (uint64_t)stack + STACK_SIZE;

    // Initialize GDT entries (same as bootstrap)
    gdt_setup(core->gdt, 0, 0, 0x0, 0x0, 0x0);      // Null
    gdt_setup(core->gdt, 1, 0, 0xFFFF, 0x9A, 0xA0); // Kernel Code
    gdt_setup(core->gdt, 2, 0, 0xFFFF, 0x92, 0xA0); // Kernel Data
    gdt_setup(core->gdt, 3, 0, 0xFFFF, 0xFA, 0xA0); // User Code
    gdt_setup(core->gdt, 4, 0, 0xFFFF, 0xF2, 0xA0); // User Data

    // Initialize TSS for this core
    memset(&core->tss, 0, sizeof(core->tss));
    core->tss.rsp0 = core->kernel_stack;
    core->tss.iopb_offset = sizeof(core->tss);

    // Add TSS descriptor to GDT
    gdt_setup_sysseg(core->gdt, 5, (uint64_t)&core->tss, sizeof(core->tss) - 1, 0x89, 0x00);

    core->gdtr.limit = (uint16_t) sizeof (core->gdt) - 1; // 7*16 - 1 = 111 bytes
    core->gdtr.base = (uint64_t) &core->gdt;

#if 0
    for (int i = 0; i < GDT_ENTRIES_COUNT; i++) {
        printf("GDT Entry %d: %x\n", i, *(uint64_t*)&core->gdt[i]);
    }
#endif

}

void load_gdt_tss(cpu_data_t *core) {
    // Load GDT
    gdt_flush(&core->gdtr);

    // Load TSS
    tss_flush(0x28);  // Selector 0x28 (5th entry in GDT)
}

void init_all_gdt_tss() {
    int num_cores = detect_cores();  // Detect available CPU cores (e.g., via ACPI)

    for (int core = 0; core < num_cores; core++)
        init_gdt_tss(&cpu_data[core]);
}


void core_init(int core) {
    // Get core-specific data
    cpu_data_t *data = &cpu_data[core];

    load_gdt_tss(data);

    // Set kernel stack in TSS
    // Michael Petch - this doesn't look right - bug???
    asm volatile("mov %0, %%rsp\n" ::"r"(data->tss.rsp0));

    printf("Successfully GDT and TSS enabled for CPU %d!\n", core);
}

void start_secondary_cores() {
    int num_cores = detect_cores();
    for (int core = 1; core < num_cores; core++) {
        lapic_send_ipi(core, 0);  // Platform-specific IPI code
        printf("Successfully GDT and TSS enabled for CPU %d!\n", core);
    }
}
