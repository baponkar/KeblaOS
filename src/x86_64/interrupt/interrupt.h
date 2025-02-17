

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../util/util.h" // for registers_t structure


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


extern void interrupt_flush(uint64_t);
extern int_entry_t int_entries[256];
extern int_ptr_t int_ptr;
extern char* exception_messages[];

extern void *interrupt_routines[16];


void disable_pic();
void interrupt_install_handler(int int_no, void (*handler)(registers_t *r));
void interrupt_uninstall_handler(int int_no);
void int_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr);

void disable_interrupts();
void enable_interrupts();
void cpu_exception_handler(registers_t *regs);

void test_interrupt();

void gpf_handler(registers_t *regs);
void debug_error_code(int err_code);


