#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../util/util.h"

static inline uint64_t rdmsr(uint32_t msr);
static inline void wrmsr(uint32_t msr, uint64_t value);

void disable_pic();
void mmio_write(uint32_t address, uint32_t value);
uint32_t mmio_read(uint32_t address);
int has_apic();
uint32_t get_lapic_id();
void enable_apic();
void apic_send_eoi();
void enable_ioapic_mode();
static inline void ioapic_write(uint32_t reg, uint32_t value);
static inline uint32_t ioapic_read(uint32_t reg);
void ioapic_remap_keyboard();
void init_apic();

void apic_irq_handler(registers_t *regs);
void apic_interrupt_install_handler(int irq_no, void (*handler)(registers_t *r));
void apic_interrupt_uninstall_handler(int int_no);


