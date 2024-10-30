
#include "gdt.h"

extern void setGdt();
gdt_entry_t gdt_entries[5];


void gdt_set_gate(int8_t index, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags){
    gdt_entries[index].limit_low = limit & 0xFFFF;   // set lower 16 bit
    gdt_entries[index].limit_up = limit & 0xF0000;   // set upper 4 bit
    
    gdt_entries[index].base_low = base & 0xFFFF;     // set lower 16 bit
    gdt_entries[index].base_mid = (base >> 16) & 0x00FF0000;   // set middle 8 bit
    gdt_entries[index].base_up = (base & 0xFF000000) >> 24;   // set upper 8 bit
     gdt_entries[index].base_top = (base & 0xFFFFFFFF00000000) >> 32; // set top 32 bit

    gdt_entries[index].access = access;     // set 8 bit
    gdt_entries[index].flags = flags;        // set 
}


void init_gdt(){
    gdt_set_gate(0,0,0x00000000,0x00,0x0);
    gdt_set_gate(1,0,0xFFFFF,0x9A,0xA); // kernel mode code segment
    gdt_set_gate(2,0,0xFFFFF,0x92,0xC); // kernel mode data segment
    gdt_set_gate(3,0,0xFFFFF,0xFA,0xA); // user mode code segment
    gdt_set_gate(4,0,0xFFFFF,0xF2,0xC); // user mode data segment

    // Calculate the GDT limit and base address
    uint16_t limit = (sizeof(gdt_entries) - 1);
    uint64_t base = (uint64_t) &gdt_entries;

    // Call setGdt with the limit and base address
    setGdt(limit, base);
}

