#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../util/util.h"

// Defined in irq
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
extern void irq13();    // FPU / Floating-Point Unit (Coprocessor), IRQ13
extern void irq14();    // Primary ATA Hard Disk Controller, IRQ14
extern void irq15();    // Secondary ATA Hard Disk Controller, IRQ15
extern void irq16();    // APIC Timer
extern void irq17();    // HPET Timer
extern void irq18(); 

extern void irq19();    // IPI (Inter-Processor Interrupt)

extern void irq96();    // System Call

void irq_handler(registers_t *regs);
void irq_install(int irq_no, void (*handler)(registers_t *r));
void irq_uninstall(int irq_no);

