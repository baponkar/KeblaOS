
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
#include "../../bootloader/cpu.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../mmu/kmalloc.h"
#include "../../lib/assert.h"
#include "../interrupt/apic.h"

#include "gdt.h"


#define STACK_SIZE 0x4000  // 16 KB
#define GDT_ENTRIES_COUNT 7 // 5 GDT(64 Bit) + 1 TSS (128 Bit)

extern void gdt_flush(gdtr_t *gdtr_instance);
extern void tss_flush(uint16_t selector);

gdt_entry_t gdt_entries_bootstrap[GDT_ENTRIES_COUNT]; // 64 bit GDT entries
tss_entry_t tss_entry_bootstrap;    // 128 bit TSS entry

gdtr_t gdtr_bootstrap;                // Global Descriptor Table Register
tss_t tss_bootstrap;

// Setup GDT entry
void gdt_setup_bootstrap(uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt_entries_bootstrap[idx].limit_low    = limit & 0xFFFF;          // Lower 16 bits of limit
    gdt_entries_bootstrap[idx].base_low     = base & 0xFFFF;           // Lower 16 bits of base
    gdt_entries_bootstrap[idx].base_middle  = (base >> 16) & 0xFF;     // Middle 8 bits of base
    gdt_entries_bootstrap[idx].access       = access;                  // Access byte
    gdt_entries_bootstrap[idx].granularity  = (limit >> 16) & 0x0F;    // Upper 4 bits of limit
    gdt_entries_bootstrap[idx].granularity |= granularity & 0xF0;      // Flags
    gdt_entries_bootstrap[idx].base_high    = (base >> 24) & 0xFF;     // Next 8 bits of base

    // printf("GDT Entry %d: Base: %x, Limit: %x, Access: %x, Granularity: %x\n", idx, base, limit, access, granularity);
}



// Setup TSS entry in GDT
void tss_setup_bootstrap(uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    tss_entry_bootstrap.limit_low = limit & 0xFFFF;
    tss_entry_bootstrap.base_low = base & 0xFFFF;
    tss_entry_bootstrap.base_middle = (base >> 16) & 0xFF;
    tss_entry_bootstrap.access = access;
    tss_entry_bootstrap.granularity = (limit >> 16) & 0x0F;
    tss_entry_bootstrap.granularity |= (granularity & 0xF0);
    tss_entry_bootstrap.base_high = (base >> 24) & 0xFF;

    tss_entry_bootstrap.base_upper = (base >> 32) & 0xFFFFFFFF;
    tss_entry_bootstrap.reserved = 0;

    // Ensure `tss_entry_t` is 16 bytes (two GDT entries)
    assert(sizeof(tss_entry_t) == 16);
    memcpy(&gdt_entries_bootstrap[5], &tss_entry_bootstrap, sizeof(tss_entry_t));

    // printf("TSS Entry %d: Base: %x, Limit: %x, Access: %x, Granularity: %x\n", idx, base, limit, access, granularity);
}


// Initialize GDT and TSS for Bootstrap CPU
void start_bootstrap_gdt_tss() {
    // Setting GDTs
    gdt_setup_bootstrap(0, 0, 0x0,    0x0,  0x0);     // Null descriptor selector : 0x0
    gdt_setup_bootstrap(1, 0, 0xFFFF, 0x9A, 0xA0);    // Kernel mode code segment, selector : 0x8
    gdt_setup_bootstrap(2, 0, 0xFFFF, 0x92, 0xA0);    // Kernel mode data segment, selector : 0x10
    gdt_setup_bootstrap(3, 0, 0xFFFF, 0xFA, 0xA0);    // User mode code segment, selector : 0x18 
    gdt_setup_bootstrap(4, 0, 0xFFFF, 0xF2, 0xA0);    // User mode data segment, selector : 0x20

    // Setting TSS
    memset(&tss_bootstrap, 0, sizeof(tss_t));
    // Allocate a kernel stack for Ring 0
    uint64_t stack = kmalloc_a(STACK_SIZE, true);
    if (!stack) {
        printf("Error: Stack allocation failed\n");
        return;
    }
    tss_bootstrap.rsp0 = (uint64_t)stack + STACK_SIZE;  // Top of stack

    // Set I/O Permission Bitmap offset (no I/O bitmap)
    tss_bootstrap.iopb_offset = sizeof(tss_t);
    // Add the TSS to GDT (selector 0x28)
    tss_setup_bootstrap(5, (uint64_t) &tss_bootstrap, sizeof(tss_t) - 1, 0x89, 0x00);

    gdtr_bootstrap.limit = (uint16_t) (GDT_ENTRIES_COUNT * sizeof(gdt_entry_t)) - 1; // 7*16 - 1 = 111 bytes
    gdtr_bootstrap.base = (uint64_t) &gdt_entries_bootstrap;  //                   

    // for (int i = 0; i < GDT_ENTRIES_COUNT; i++) {
    //     printf("GDT Entry %d: %x\n", i, *(uint64_t*)&gdt_entries_bootstrap[i]);
    // }

    
    gdt_flush((gdtr_t *) &gdtr_bootstrap);
    // printf("gdt flush completed.\n");

    tss_flush(0x28);
    // printf("tss flush completed.\n");

    printf("Successfully started GDT and TSS for Bootstrap CPU.\n");
}







#define MAX_CORES 256

typedef struct {
    gdt_entry_t gdt[GDT_ENTRIES_COUNT];  // Each core's GDT
    tss_entry_t tss_entry;                // TSS descriptor in GDT
    tss_t tss;                            // Core's Task State Segment
    uint64_t kernel_stack;                // Kernel stack for Ring 0
} cpu_data_t;

cpu_data_t cpu_data[MAX_CORES];  // Array indexed by CPU ID (APIC ID)

int detect_cores(){
    return get_cpu_count();
}

void gdt_setup(int core, uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    cpu_data_t *data = &cpu_data[core];
    data->gdt[idx].limit_low = limit & 0xFFFF;
    data->gdt[idx].base_low = base & 0xFFFF;
    data->gdt[idx].base_middle = (base >> 16) & 0xFF;
    data->gdt[idx].access = access;
    data->gdt[idx].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
    data->gdt[idx].base_high = (base >> 24) & 0xFF;
}

void tss_setup(int core, uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    cpu_data_t *data = &cpu_data[core];
    // Populate TSS descriptor (similar to gdt_setup)
    data->tss_entry.limit_low = limit & 0xFFFF;
    data->tss_entry.base_low = base & 0xFFFF;
    data->tss_entry.base_middle = (base >> 16) & 0xFF;
    data->tss_entry.access = access;
    data->tss_entry.granularity = (limit >> 16) & 0x0F;
    data->tss_entry.granularity |= (granularity & 0xF0);
    data->tss_entry.base_high = (base >> 24) & 0xFF;
    data->tss_entry.base_upper = (base >> 32) & 0xFFFFFFFF;
    data->tss_entry.reserved = 0;

    memcpy(&data->gdt[5], &data->tss_entry, sizeof(tss_entry_t));
}

void init_all_gdt_tss() {
    int num_cores = detect_cores();  // Detect available CPU cores (e.g., via ACPI)

    for (int core = 0; core < num_cores; core++) {
        // Allocate kernel stack
        uint64_t stack = kmalloc_a(STACK_SIZE, true);
        cpu_data[core].kernel_stack = (uint64_t)stack + STACK_SIZE;

        // Initialize GDT entries (same as bootstrap)
        gdt_setup(core, 0, 0, 0x0, 0x0, 0x0);      // Null
        gdt_setup(core, 1, 0, 0xFFFF, 0x9A, 0xA0); // Kernel Code
        gdt_setup(core, 2, 0, 0xFFFF, 0x92, 0xA0); // Kernel Data
        gdt_setup(core, 3, 0, 0xFFFF, 0xFA, 0xA0); // User Code
        gdt_setup(core, 4, 0, 0xFFFF, 0xF2, 0xA0); // User Data

        // Initialize TSS for this core
        memset(&cpu_data[core].tss, 0, sizeof(tss_t));
        cpu_data[core].tss.rsp0 = cpu_data[core].kernel_stack;
        cpu_data[core].tss.iopb_offset = sizeof(tss_t);

        // Add TSS descriptor to GDT
        tss_setup(core, 5, (uint64_t)&cpu_data[core].tss, sizeof(tss_t) - 1, 0x89, 0x00);
    }
}


void core_init(int core) {
    // Get core-specific data
    cpu_data_t *data = &cpu_data[core];

    // Load GDT
    gdtr_t gdtr;
    gdtr.limit = sizeof(gdt_entry_t) * GDT_ENTRIES_COUNT - 1;
    gdtr.base = (uint64_t)data->gdt;
    gdt_flush(&gdtr);

    // Load TSS
    tss_flush(0x28);  // Selector 0x28 (5th entry in GDT)

    // Set kernel stack in TSS
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
