
#include "../../memory/kmalloc.h"
#include "../../lib/stdio.h"
#include "gdt.h"

#include "tss.h"

#define GDT_ENTRIES 7
#define STACK_SIZE 0x1000   // 16 KB

extern gdt_entry_t gdt[GDT_ENTRIES];  // Defined in gdt.c
tss_t tss;

void tss_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].limit_low     = (limit & 0xFFFF);
    gdt[i].base_low      = (base & 0xFFFF);
    gdt[i].base_middle   = (base >> 16) & 0xFF;
    gdt[i].access        = access;
    gdt[i].granularity   = (limit >> 16) & 0x0F;
    gdt[i].granularity  |= gran & 0xF0;             // flags = 0
    gdt[i].base_high     = (base >> 24) & 0xFF;

    // Populate the upper 64-bits of the system segment descriptor
    gdt[i + 1].base_upper = (base >> 32) & 0xFFFFFFFF;
    gdt[i + 1].reserved = 0;
}

void tss_init(){
    uint64_t stack = (uint64_t) kmalloc_a(STACK_SIZE, 1);  // 4KB aligned
    if(!stack){
        printf("Stack Memory Creation Failed!\n");
        return;
    }

    tss.rsp0 = stack + STACK_SIZE;  // Set Stack Top
    tss.iopb_offset = sizeof(tss_t);

    tss_set_entry(5, (uint32_t)&tss, (sizeof(tss_t) - 1) , 0x89, 0x00);     // TSS Descriptor, selector 0x28
}


void print_tss(tss_t *tss) {
    printf("TSS RSP0: %x\n", tss->rsp0);
    printf("TSS IOPB: %x\n", tss->iopb_offset);
}
// Example Usage
// print_tss(&tss);

