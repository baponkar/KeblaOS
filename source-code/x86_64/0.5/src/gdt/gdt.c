
/*
Reference :
https://wiki.osdev.org/Global_Descriptor_Table
https://wiki.osdev.org/GDT_Tutorial
https://web.archive.org/web/20160326064709/http://jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/04_GDT.md
*/

#include "gdt.h"

extern void gdt_flush(uint32_t, uint64_t);
extern void reloadSegments();

gdt_entry_t gdt_entries[5];

void gdt_setup( int idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity){
    
    // base address 64 bit
    gdt_entries[idx].base_low     = base         & 0xFFFF;     // 16 bit
    gdt_entries[idx].base_middle  = (base >> 16) & 0xFF;       // 8 bit
    gdt_entries[idx].base_high    = (base >> 24) & 0xFF;       // 8 bit
    gdt_entries[idx].base_upper   = (base >> 32) & 0xFFFFFFFF; // 32 bit

    gdt_entries[idx].access       = access;

    // limit 20 bit
    gdt_entries[idx].limit_low    = limit & 0xFFFF; // 16 bit

    gdt_entries[idx].granularity  = (limit >> 16) & 0x0F;    // Set limit : lower 4 bit
    gdt_entries[idx].granularity |= granularity & 0xF0;     // Set Flags : upper 4 bit

    gdt_entries[idx].reserved     = 0x00000000; // 32 bit
}


void init_gdt(){
    
    gdt_setup(0, 0, 0x0, 0x0, 0x0);          // null descriptor selector : 0x0
    // index = 1, base = 0, limit = 0xFFFFFFFF, access = 0x9A, granularity = 0xA0
    gdt_setup(1, 0, 0xFFFFFFFF, 0x9A, 0xA0);    // kernel mode code segment, selector : 0x8

    // index = 2, base = 0, limit = 0xFFFFFFFF, access = 0x92, granularity = 0xA0
    gdt_setup(2, 0, 0xFFFFFFFF, 0x92, 0xA0);    // kernel mode data segment, selector : 0x10

    // index = 3, base = 0, limit = 0xFFFFFFFF, access = 0xFA, granularity = 0xA0
    gdt_setup(3, 0, 0xFFFFFFFF, 0xFA, 0xA0);    // user mode code segment, selector : 0x18 

    // index = 4, base = 0, limit = 0xFFFFFFFF, access = 0xF2, granularity = 0xA0 
    gdt_setup(4, 0, 0xFFFFFFFF, 0xF2, 0xA0);    // user mode data segment, selector : 0x20

    // Calculate the GDT limit and base address
    uint32_t limit = (uint32_t) (sizeof(gdt_entry_t) * 3 - 1);
    uint64_t base = (uint64_t) &gdt_entries;

    gdt_flush(limit, base);
    //reloadSegments();
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

