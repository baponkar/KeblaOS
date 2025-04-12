
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

#include "tss.h"

#include "gdt.h"


/*
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
*/

#define STACK_SIZE 0x4000   // 16 KB
#define GDT_ENTRIES 7       // 1-Null(64 Bit) + 2-Kernel(2*64 Bit) + 2-User(2*64 Bit) + 1-TSS(128 Bit)

extern void gdt_flush(gdtr_t *gdtr);
extern void tss_flush(uint16_t selector);

gdt_entry_t gdt[GDT_ENTRIES];
gdtr_t gdtr;

// granularity(8 Bit)=> Flags(Upper 4 Bit) | Up-Limit(Lower 4 Bit)
// flags = (granularity & 0xF0) >> 4
// up_limit = granularity & 0xF
void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].limit_low     = (limit & 0xFFFF);
    gdt[i].base_low      = (base & 0xFFFF);
    gdt[i].base_middle   = (base >> 16) & 0xFF;
    gdt[i].access        = access;
    gdt[i].granularity   = (limit >> 16) & 0x0F;
    gdt[i].granularity  |= gran & 0xF0;
    gdt[i].base_high     = (base >> 24) & 0xFF;
}

// Access Byte                Flags       
//      P DPL S E DC RW A      G DB L R
// 92 = 1 00  1 0 0  1  0  A = 1 0  1 0
// 93 = 1 00  1 0 0  1  1  C = 1 1  0 0
void gdt_init(){
    gdt_set_entry(0, 0, 0, 0, 0);              // Null Descriptor
    gdt_set_entry(1, 0, 0xFFFF, 0x9A, 0xA0);   // Kernel Code Descriptor , Selector 0x08
    gdt_set_entry(2, 0, 0xFFFF, 0x92, 0xA0);   // Kernel Data Descriptor , Selector 0x10
    gdt_set_entry(3, 0, 0xFFFF, 0xFA, 0xA0);   // User Code Descriptor , Selector 0x18
    gdt_set_entry(4, 0, 0xFFFF, 0xF2, 0xA0);   // User Data Descriptor , Selector 0x20

    // gdtr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    // gdtr.base = (uint64_t) &gdt;
}

void gdt_tss_init(){
    gdt_init();
    tss_init();

    gdtr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    gdtr.base = (uint64_t) &gdt;

    gdt_flush(&gdtr);     // Load GDT
    tss_flush(0x28);      // Selector 0x28 (5th entry in GDT)
    // asm volatile("ltr %0" : : "r"(0x28));

    printf("[Info] GDT & TSS initialized.\n");
}


void print_gdt_entry(uint16_t selector) {
    uint64_t gdt_base = (uint64_t) &gdt;
    size_t index = selector >> 3;
    gdt_entry_t *entry = (gdt_entry_t*)(gdt_base + index * sizeof(gdt_entry_t));

    uint32_t base = entry->base_high << 24 | entry->base_middle << 16 | entry->base_low;
    uint32_t limit = (entry->granularity & 0xF) << 16 | entry->limit_low;
    uint8_t access = entry->access;
    uint8_t flags = (entry->granularity & 0xF0) >> 4;

    printf("GDT Entry %x: Base=%x Limit=%x Access=%x Flags=%x\n",
           selector, base, limit, access, flags);
}


// Example usage:
// print_gdt_entry(0);     // Null Descriptor
// print_gdt_entry(0x08);  // Kernel code segment
// print_gdt_entry(0x10);  // Kernel data segment
// print_gdt_entry(0x1B);  // User code segment
// print_gdt_entry(0x23);  // User data segment
// print_gdt_entry(0x28);  // TSS segment











