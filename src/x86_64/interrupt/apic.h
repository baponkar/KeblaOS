#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

static inline uint64_t rdmsr(uint32_t msr);
static inline void wrmsr(uint32_t msr, uint64_t value);

void mmio_write(uintptr_t address, uint64_t value);
uint64_t mmio_read(uintptr_t address);

void enable_apic();
void apic_remap_timer();
void apic_send_eoi();
void enable_ioapic_mode();

void init_apic();
void disable_pic();
