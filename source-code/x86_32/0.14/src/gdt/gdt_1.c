
#include "gdt.h"


gdt_entry_t gdt_entries[5];

void gdt_set_gate(int8_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran){
    gdt_entries[index].limit_low = limit & 0xFFFF;   // set lower 16 bit
    gdt_entries[index].limit_up = limit & 0xF0000;   // set upper 4 bit
    
    gdt_entries[index].base_low = base & 0xFFFF;     // set lower 16 bit
    gdt_entries[index].base_mid = (base >> 16) & 0x00FF0000;   // set middle 8 bit
    gdt_entries[index].base_up = (base & 0xFF000000) >> 24;   // set upper 8 bit

    gdt_entries[index].access = access;     // set 8 bit
    gdt_entries[index].flags = gran;        // set 
}


