#pragma once

#include <stdint.h>

struct idt_entry_struct
{//128 bit
    uint16_t offset_1;        // offset bits 0..15, 16 bit
    uint16_t selector;        // a code segment selector in GDT or LDT, 16 bit
    uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.2 bit
    uint8_t  type_attributes; // gate type, dpl, and p fields , 8 bit
    uint16_t offset_2;        // offset bits 16..31 , 16 bit

    uint32_t offset_3;        // offset bits 32..63 , 32 bit
    uint32_t zero;            // reserved, 
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;



// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
   uint16_t limit;
   uint64_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;



void idt_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr);
void isr_install();
typedef struct registers registers_t;
void isr_handler(registers_t *regs);

void debug_error_code(int err_code);

void interrupt_install_handler(int int_no, void (*handler)(registers_t *r));
void interrupt_uninstall_handler(int int_no);

void init_idt();
void test_interrupt();

void irq_remap(void);
void irq_install();
void irq_handler(registers_t *r);

void disable_interrupts();
void enable_interrupts();

void gpf_handler(registers_t *regs);




