#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

extern bool has_apic(); // defined in cpuid.c 
void apic_write(uint32_t reg, uint32_t value);
uint32_t apic_read(uint32_t reg);

void enable_apic();
uint32_t get_lapic_id();
void apic_send_eoi();
void lapic_send_ipi(uint8_t cpu_id, uint8_t vector);

void init_apic_interrupt();

