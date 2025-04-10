
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
#include "../../memory/kmalloc.h"
#include "../../lib/assert.h"
#include "../interrupt/apic.h"
#include "../../util/util.h"

#include "gdt.h"

#define MAX_CORES 256
#define STACK_SIZE 0x4000  // 16 KB

extern void gdt_flush(gdtr_t *gdtr_instance);
extern void tss_flush(uint16_t selector);

cpu_data_t cpu_data[MAX_CORES];  // Array indexed by CPU ID (APIC ID)


// Detect the number of CPU cores available
int detect_cores(){
    return get_cpu_count();
}

// Setup a GDT entry
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

// Initialize GDT and TSS for a core
void set_core_gdt_tss(int core_id) {
    if(core_id >= MAX_CORES) {
        printf("Invalid core ID %d\n", core_id);
        return;
    }

    cpu_data_t *core = &cpu_data[core_id];
    uint64_t stack = 0;

    if(core_id == 0){
        stack = (uint64_t) read_rsp();          // Using present bootstrap cpu stack
    }else{
        stack = kmalloc_a(STACK_SIZE, true);    // Assigning new stack memories for application cores
    }
    
    
    if(stack == 0) {
        printf("Failed to allocate kernel stack for CPU %d\n", core_id);
        return;
    }

    core->kernel_stack = (uint64_t)stack + STACK_SIZE;

    // Initialize GDT entries (same as bootstrap)
    gdt_setup(core->gdt, 0, 0, 0x0, 0x0, 0x0);      // Null
    gdt_setup(core->gdt, 1, 0, 0xFFFF, 0x9A, 0xA0); // Kernel Code Selector 0x08
    gdt_setup(core->gdt, 2, 0, 0xFFFF, 0x92, 0xA0); // Kernel Data Selector 0x10
    gdt_setup(core->gdt, 3, 0, 0xFFFF, 0xFA, 0xA0); // User Code Selector   0x18
    gdt_setup(core->gdt, 4, 0, 0xFFFF, 0xF2, 0xA0); // User Data Selector   0x20

    // Initialize TSS for this core
    memset(&core->tss, 0, sizeof(core->tss));
    core->tss.rsp0 = core->kernel_stack;
    core->tss.iopb_offset = sizeof(core->tss);

    // Add TSS descriptor to GDT
    gdt_setup_sysseg(core->gdt, 5, (uint64_t)&core->tss, sizeof(core->tss) - 1, 0x89, 0x00);

    core->gdtr.limit = (uint16_t) sizeof (core->gdt) - 1; // 7*16 - 1 = 111 bytes
    core->gdtr.base = (uint64_t) &core->gdt;
}


// Load GDT and TSS for a core
void load_core_gdt_tss(int core_id) {
    cpu_data_t *core = &cpu_data[core_id];
    gdt_flush(&core->gdtr); // Load GDT

    // Load TSS
    tss_flush(0x28);  // Selector 0x28 (5th entry in GDT)
}

// Initialize GDT and TSS for a single core
void init_core_gdt_tss(int core_id) {
    set_core_gdt_tss(core_id);
    load_core_gdt_tss(core_id);
}

void init_bootstrap_gdt_tss(int bootstrap_core_id) {
    set_core_gdt_tss(bootstrap_core_id);
    load_core_gdt_tss(bootstrap_core_id);
}

// Initialize GDT and TSS for all cores
void init_application_core_gdt_tss(int start_core_id, int end_core_id) {
    for(int i = start_core_id; i <= end_core_id; i++) {
        init_core_gdt_tss(i);
    }
}






