#pragma once

#include <stdint.h>

#include "../../util/util.h"

extern void pic_flush(uint64_t);

extern void pic_irq0();
extern void pic_irq1();
extern void pic_irq2();
extern void pic_irq3();
extern void pic_irq4();
extern void pic_irq5();
extern void pic_irq6();
extern void pic_irq7();
extern void pic_irq8();
extern void pic_irq9();
extern void pic_irq10();
extern void pic_irq11();
extern void pic_irq12();
extern void pic_irq13();
extern void pic_irq14();
extern void pic_irq15();

extern void pic_isr0();
extern void pic_isr1();
extern void pic_isr2();
extern void pic_isr3();
extern void pic_isr4();
extern void pic_isr5();
extern void pic_isr6();
extern void pic_isr7();
extern void pic_isr8();
extern void pic_isr9();
extern void pic_isr10();
extern void pic_isr11();
extern void pic_isr12();
extern void pic_isr13();
extern void pic_isr14();
extern void pic_isr15();
extern void pic_isr16();
extern void pic_isr17();
extern void pic_isr18();
extern void pic_isr19();
extern void pic_isr20();
extern void pic_isr21();
extern void pic_isr22();
extern void pic_isr23();
extern void pic_isr24();
extern void pic_isr25();
extern void pic_isr26();
extern void pic_isr27();
extern void pic_isr28();
extern void pic_isr29();
extern void pic_isr30();
extern void pic_isr31();

extern void pic_isr128();
extern void pic_isr177();

void pic_isr_handler(registers_t *regs);
void pic_isr_install();

void pic_irq_handler(registers_t *regs);
void pic_irq_remap();

void pic_irq_install();
void init_pic_interrupt();




