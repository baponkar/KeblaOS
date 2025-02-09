#pragma once

#include <stdint.h>

struct apic_entry_struct
{//128 bit
    uint16_t offset_1;        // offset bits 0..15, 16 bit
    uint16_t selector;        // a code segment selector in GDT or LDT, 16 bit
    uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.2 bit
    uint8_t  type_attributes; // gate type, dpl, and p fields , 8 bit
    uint16_t offset_2;        // offset bits 16..31 , 16 bit

    uint32_t offset_3;        // offset bits 32..63 , 32 bit
    uint32_t zero;            // reserved, 
} __attribute__((packed));
typedef struct apic_entry_struct apic_entry_t;



// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct apic_ptr_struct
{
   uint16_t limit;
   uint64_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct apic_ptr_struct apic_ptr_t;


