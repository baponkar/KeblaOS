
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
    gdt[i].limit_low     = (limit & 0xFFFF);        // Set lower 16 bit of limit
    gdt[i].granularity   = (limit >> 16) & 0x0F;    // Set upper 4 bit of limit

    gdt[i].base_low      = (base & 0xFFFF);         // Set lower 16 bit baase
    gdt[i].base_middle   = (base >> 16) & 0xFF;     // Set middle 8 bit base
    gdt[i].base_high     = (base >> 24) & 0xFF;     // Set high 8 bit base

    gdt[i].access        = access;                  // Set  bit access

    gdt[i].granularity  |= gran & 0xF0;             // Set 4 bit Flags
    
}

// Access Byte                      Flags       
//      P DPL S E DC RW A           G DB L R
// 92 = 1 00  1 0 0  1  0       A = 1 0  1 0 = 1010 
// 93 = 1 00  1 0 0  1  1       C = 1 1  0 0 = 1100 
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

    printf(" [-] GDT & TSS initialized.\n");
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
// print_gdt_entry(0);     // Null Descriptor     : GDT Entry 0x0:  Base=0x0 Limit=0x0    Access=0x0  Flags=0x0
// print_gdt_entry(0x08);  // Kernel code segment : GDT Entry 0x8:  Base=0x0 Limit=0xFFFF Access=0x9A Flags=0xA
// print_gdt_entry(0x10);  // Kernel data segment : GDT Entry 0x10: Base=0x0 Limit=0xFFFF Access=0x93 Flags=0x8
// print_gdt_entry(0x1B);  // User code segment   : GDT Entry 0x1B: Base=0x0 Limit=0xFFFF Access=0xFA Flags=0xA
// print_gdt_entry(0x23);  // User data segment   : GDT Entry 0x23: Base=0x0 Limit=0xFFFF Access=0xF2 Flags=0xA
// print_gdt_entry(0x28);  // TSS segment         : GDT Entry 0x28: Base=0x8059A060 Limit=0x67 Access=0x8B Flags=0x0











