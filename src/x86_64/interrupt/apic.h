#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

uint64_t rdmsr(uint32_t msr);
void wrmsr(uint32_t msr, uint64_t value);

void disable_pic();
void mmio_write(uintptr_t address, uint64_t value);
uint64_t mmio_read(uintptr_t address);
int has_apic();
uint32_t get_lapic_id();
void enable_apic();
void apic_send_eoi();
void enable_ioapic_mode();
static inline void ioapic_write(uint32_t reg, uint32_t value);
static inline uint32_t ioapic_read(uint32_t reg);
void ioapic_remap_keyboard();
void init_apic();



