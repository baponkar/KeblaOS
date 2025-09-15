
#include "../../memory/kmalloc.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "gdt.h"

#include "tss.h"

extern bool debug_on;

#define GDT_ENTRIES 7
#define STACK_SIZE 0x1000               // 16 KB

extern gdt_entry_t gdt[GDT_ENTRIES];  // Defined in gdt.c
tss_t tss;

void tss_set_entry(int i, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].limit_low     = (limit & 0xFFFF);
    gdt[i].granularity   = (limit >> 16) & 0x0F;

    gdt[i].base_low      = (base & 0xFFFF);             // Lower 16-1 Bit from base
    gdt[i].base_middle   = (base >> 16) & 0xFF;         // 24-17 bit from base
    gdt[i].base_high     = (base >> 24) & 0xFF;         // 32-25 bit from base

    gdt[i].access        = access;                      // Set 8 bit access
    
    gdt[i].granularity  |= gran & 0xF0;                 // Set 4 Bit Flags
    
    // Populate the upper 64-bits of the system segment descriptor
    gdt[i + 1].base_upper = (base >> 32) & 0xFFFFFFFF;  // 64-33 Bit from base
    gdt[i + 1].reserved = 0;
}

void tss_init(){
    uint64_t stack = (uint64_t) kmalloc_a(STACK_SIZE, 1);  // 4KB aligned
    if(!stack){
        if(debug_on) printf("Stack Memory Creation Failed!\n");
        return;
    }
    memset(&tss, 0, sizeof(tss_t));

    tss.rsp0 = stack + STACK_SIZE;  // Set Stack Top
    tss.iopb_offset = sizeof(tss_t);

    // TSS descriptor needs two entries (16 bytes)
    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = (uint32_t)(sizeof(tss_t));

    // Access Byte                      Flags       
    //      P DPL S E DC RW A           G DB L R
    // 92 = 1 00  1 0 0  1  0       A = 1 0  1 0 
    // 93 = 1 00  1 0 0  1  1       C = 1 1  0 0
    // 89 = 1 00  0 1 0  0  1       0 = 0 0  0 0
    tss_set_entry(5, tss_base, tss_limit , 0x89, 0x00);     // TSS Descriptor, selector 0x28
}


void print_tss(tss_t *tss) {
    printf("TSS RSP0: %x\n", tss->rsp0);
    printf("TSS IOPB: %x\n", tss->iopb_offset);
}
// Example Usage
// print_tss(&tss);

