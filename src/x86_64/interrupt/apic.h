#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>




void mmio_write(uint32_t address, uint32_t value); // 
uint32_t mmio_read(uint32_t address);

int has_apic();
uint32_t get_lapic_id();

void lapic_send_ipi(uint8_t cpu_id, uint8_t vector);

static inline uint64_t rdmsr(uint32_t msr);
static inline void wrmsr(uint32_t msr, uint64_t value);

void enable_apic();
void apic_send_eoi();

void enable_ioapic_mode();

void init_apic_interrupt();



