

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


extern void idt_flush(uint64_t);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr128();
extern void isr177();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void irq16();


extern void interrupt_flush(uint64_t);
extern int_entry_t int_entries[256];
extern int_ptr_t int_ptr;
extern char* exception_messages[];

extern void (*interrupt_routines[224])(registers_t *);


void disable_pic();
void interrupt_install_handler(int int_no, void (*handler)(registers_t *r));
void interrupt_uninstall_handler(int int_no);
void int_set_gate(uint8_t index, uint64_t offset, uint16_t selector, uint8_t attr);

void disable_interrupts();
void enable_interrupts();
void cpu_exception_handler(registers_t *regs);

void set_descriptor_table();
void init_interrupt();

void test_interrupt();

void gpf_handler(registers_t *regs);
void debug_error_code(int err_code);


