/*
    Global Descriptor Table

    https://wiki.osdev.org/Global_Descriptor_Table
    https://wiki.osdev.org/GDT_Tutorial
    https://web.archive.org/web/20160326064709/http://jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
    http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial
    https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/04_GDT.md
    https://stackoverflow.com/questions/79260987/why-gdt-is-not-working-in-the-x86-64-system
    https://forum.osdev.org/viewtopic.php?t=57521
*/

#include "gdt.h"


extern void gdt_flush(gdtr_t *gdtr_instance);
extern void reloadSegments();

gdt_entry_t gdt_entries[5];
gdtr_t gdtr_instance;

void gdt_setup( uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity){
    gdt_entries[idx].limit_low    = limit & 0xFFFF;       // 16 bit
    gdt_entries[idx].base_low     = base  & 0xFFFF;       // 16 bit
    gdt_entries[idx].base_middle  = (base >> 16) & 0xFF;  // 8 bit
    gdt_entries[idx].access       = access;
    gdt_entries[idx].granularity  = (limit >> 16) & 0x0F; // Set limit : lower 4 bit
    gdt_entries[idx].granularity |= granularity & 0xF0;   // Set Flags : upper 4 bit
    gdt_entries[idx].base_high    = (base >> 24) & 0xFF;       // 8 bit
}

// Version to create TSS and LGDT entries which require 2 GDT entries
void gdt_setup_sysseg( uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity){
    // First half of a system segment is the same as a regular segment
    gdt_setup(idx, base, limit, access, granularity);

    // Setup the following GDT entry with the upper 32 bits of the base and zero upper 32bits
    // that are reserved.
    gdt_setup(idx + 1, (base >> 48) & 0xffff , (base >> 32) & 0xffff, 0, 0);

    // The line above is equivalent to:
    // gdt_entries[idx+1].limit_low  = (base >> 32) & 0xFFFF; // lower 16 bits of the upper 32 bits of base
    // gdt_entries[idx+1].base_low   = (base >> 48) & 0xFFFF; // upper 16 bits of the upper 32 bits of base
    // gdt_entries[idx+1].base_middle  = 0;                   // Set the rest of fields to 0 (reserved)
    // gdt_entries[idx+1].access       = 0;
    // gdt_entries[idx+1].granularity  = 0;
    // gdt_entries[idx+1].base_high    = 0;
}


void init_gdt(){
    // index = 0, base = 0, limit = 0, access = 0, granularity = 0
    gdt_setup(0, 0, 0x0, 0x0, 0x0);             // null descriptor selector : 0x0
    // index = 1, base = 0, limit = 0xFFFFFFFF, access = 0x9A = 154, granularity = 0xA0 = 160
    gdt_setup(1, 0, 0xFFFFF, 0x9A, 0xA0);    // kernel mode code segment, selector : 0x8

    // index = 2, base = 0, limit = 0xFFFFFFFF, access = 0x92 = 146, granularity = 0xA0 = 160
    gdt_setup(2, 0, 0xFFFFF, 0x92, 0xA0);    // kernel mode data segment, selector : 0x10

    // index = 3, base = 0, limit = 0xFFFFFFFF, access = 0xFA = 250, granularity = 0xA0 = 160
    gdt_setup(3, 0, 0xFFFFF, 0xFA, 0xA0);    // user mode code segment, selector : 0x18 

    // index = 4, base = 0, limit = 0xFFFFFFFF, access = 0xF2 = 242, granularity = 0xA0 = 160
    gdt_setup(4, 0, 0xFFFFF, 0xF2, 0xA0);    // user mode data segment, selector : 0x20

    // Calculate the GDT limit and base address
    gdtr_instance.limit = (uint16_t) (sizeof(gdt_entries) - 1);
    gdtr_instance.base = (uint64_t) &gdt_entries;

    gdt_flush((gdtr_t *) &gdtr_instance);
    reloadSegments();

    print("Successfully GDT Enabled!\n");
}


void check_gdt() {
    uint16_t cs, ds, es, fs, gs, ss;

    // Inline assembly to get the values of segment registers
    asm volatile("mov %%cs, %0" : "=r"(cs));    
    asm volatile("mov %%ds, %0" : "=r"(ds));
    asm volatile("mov %%es, %0" : "=r"(es));
    asm volatile("mov %%fs, %0" : "=r"(fs));
    asm volatile("mov %%gs, %0" : "=r"(gs));
    asm volatile("mov %%ss, %0" : "=r"(ss));

    // Print the segment register values
    print("GDT Set\n");
    print("Segment Registers Values :\n");

    print("CS: ");
    print_hex(cs);
    print("\n");

    print("ES: ");
    print_hex(es);
    print("\n");

    print("FS: ");
    print_hex(fs);
    print("\n");

    print("GS: ");
    print_hex(gs);
    print("\n");

    print("SS: ");
    print_hex(ss);
    print("\n");
}



