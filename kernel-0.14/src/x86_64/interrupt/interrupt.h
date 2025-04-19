#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


struct int_entry_struct
{//128 bit
    uint16_t offset_1;        // offset bits 0..15, 16 bit
    uint16_t selector;        // a code segment selector in GDT or LDT, 16 bit
    uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.2 bit
    uint8_t  type_attributes; // gate type, dpl, and p fields , 8 bit
    uint16_t offset_2;        // offset bits 16..31 , 16 bit

    uint32_t offset_3;        // offset bits 32..63 , 32 bit
    uint32_t zero;            // reserved, 
} __attribute__((packed));
typedef struct int_entry_struct int_entry_t;


struct int_ptr_struct
{
   uint16_t limit;
   uint64_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct int_ptr_struct int_ptr_t;



void int_init();

