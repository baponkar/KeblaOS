
// https://wiki.osdev.org/Interrupt_Descriptor_Table
// https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/05_InterruptHandling.md

#pragma once

#include <stdint.h>
#include "../util/util.h"
#include "../driver/vga.h"

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
void isr_handler(registers_t regs);
void interrupt_install_handler(int int_no, void (*handler)(registers_t *r));
void interrupt_uninstall_handler(int int_no);
void init_idt();
void test_interrupt();

// Externel functions from ASM
extern void idt_flush(uint32_t);
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




