#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../util/util.h"

extern void apic_irq0();
extern void apic_irq1();
extern void apic_irq2();
extern void apic_irq3();
extern void apic_irq4();
extern void apic_irq5();
extern void apic_irq6();
extern void apic_irq7();
extern void apic_irq8();
extern void apic_irq9();
extern void apic_irq10();
extern void apic_irq11();
extern void apic_irq12();
extern void apic_irq13();
extern void apic_irq14();
extern void apic_irq15();

extern void apic_isr0();
extern void apic_isr1();
extern void apic_isr2();
extern void apic_isr3();
extern void apic_isr4();
extern void apic_isr5();
extern void apic_isr6();
extern void apic_isr7();
extern void apic_isr8();
extern void apic_isr9();
extern void apic_isr10();
extern void apic_isr11();
extern void apic_isr12();
extern void apic_isr13();
extern void apic_isr14();
extern void apic_isr15();
extern void apic_isr16();
extern void apic_isr17();
extern void apic_isr18();
extern void apic_isr19();
extern void apic_isr20();
extern void apic_isr21();
extern void apic_isr22();
extern void apic_isr23();
extern void apic_isr24();
extern void apic_isr25();
extern void apic_isr26();
extern void apic_isr27();
extern void apic_isr28();
extern void apic_isr29();
extern void apic_isr30();
extern void apic_isr31();
extern void apic_isr128();
extern void apic_isr177();

void mmio_write(uint32_t address, uint32_t value);
uint32_t mmio_read(uint32_t address);
int has_apic();
uint32_t get_lapic_id();
void lapic_send_ipi(uint8_t cpu_id, uint8_t vector);
static inline uint64_t rdmsr(uint32_t msr);
static inline void wrmsr(uint32_t msr, uint64_t value);
void enable_apic();
void apic_send_eoi();
void enable_ioapic_mode();
static inline void ioapic_write(uint32_t reg, uint32_t value);
static inline uint32_t ioapic_read(uint32_t reg);
void ioapic_remap_keyboard();
void ioapic_remap_timer();
void cpu_exception_install();
void apic_irq_install();
void apic_irq_handler(registers_t *regs);
void init_apic_interrupt();
void apic_isr_handler(registers_t *regs);




