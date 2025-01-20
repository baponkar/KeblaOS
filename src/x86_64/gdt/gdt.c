
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

gdt_entry_t gdt_entries[5]; // 5 GDT entries
gdtr_t gdtr_instance; // Global Descriptor Table Register


// Setup GDT entry
void gdt_setup( uint8_t idx, uint64_t base, uint32_t limit, uint8_t access, uint8_t granularity){
    gdt_entries[idx].limit_low    = limit & 0xFFFF;       // 16 bit
    gdt_entries[idx].base_low     = base  & 0xFFFF;       // 16 bit
    gdt_entries[idx].base_middle  = (base >> 16) & 0xFF;  // 8 bit
    gdt_entries[idx].access       = access;
    gdt_entries[idx].granularity  = (limit >> 16) & 0x0F; // Set limit : lower 4 bit
    gdt_entries[idx].granularity |= granularity & 0xF0;   // Set Flags : upper 4 bit
    gdt_entries[idx].base_high    = (base >> 24) & 0xFF;  // 8 bit
}

void print_gdt_entry(gdt_entry_t *entry) {
    print("Limit Low: " );
    print_hex((uint64_t) entry->limit_low);
    print("\n");
    print("Base Low: ");
    print_hex((uint64_t) entry->base_low);
    print("\n");
    print("Base Middle: ");
    print_hex((uint64_t) entry->base_middle);
    print("\n");
    print("Access: " );
    print_hex((uint64_t) entry->access);
    print("\n");
    print("Granularity: ");
    print_hex((uint64_t) entry->granularity);
    print("\n");
    print("Base High: ");
    print_hex((uint64_t) entry->base_high);
    print("\n");
}


void init_gdt(){
    print("Start of GDT initializtion...\n");
    //gdt_setup(index, base, limit, access, granularity)
    gdt_setup(0, 0, 0x0,    0x0,  0x0);     // null descriptor selector : 0x0
    gdt_setup(1, 0, 0xFFFF, 0x9A, 0xA0);    // kernel mode code segment, selector : 0x8
    gdt_setup(2, 0, 0xFFFF, 0x92, 0xA0);    // kernel mode data segment, selector : 0x10
    gdt_setup(3, 0, 0xFFFF, 0xFA, 0xA0);    // user mode code segment, selector : 0x18 
    gdt_setup(4, 0, 0xFFFF, 0xF2, 0xA0);    // user mode data segment, selector : 0x20

    // Print GDT entries for debugging
    // for (int i = 0; i < 5; i++) {
    //     print("GDT Entry : ");
    //     print_dec(i);
    //     print("\n");
    //     print_gdt_entry(&gdt_entries[i]);
    //     print("\n");
    // }

    // Calculate the GDT limit and base address
    gdtr_instance.limit = (uint16_t) (5 * sizeof(gdt_entry_t) - 1); // sizeof(gdt_entries) = sizeof(gdt_entry_t) * 5 = 40
    gdtr_instance.base = (uint64_t) &gdt_entries;

    // Print GDTR for debugging
    // print("GDTR Limit: ");
    // print_hex(gdtr_instance.limit);
    // print("\n");
    // print("GDTR Base: ");
    // print_hex(gdtr_instance.base);
    // print("\n");

    gdt_flush((gdtr_t *) &gdtr_instance);

    print("Successfully GDT initialized.\n");
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



