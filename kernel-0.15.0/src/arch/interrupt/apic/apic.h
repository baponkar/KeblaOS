#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

extern bool has_apic(); // defined in cpuid.c 

void apic_write(uint32_t reg, uint32_t value);
uint32_t apic_read(uint32_t reg);

void apic_send_eoi();

uint64_t get_lapic_base();
uint32_t get_lapic_id();
uint32_t get_lapic_version();

bool is_apic_enabled();
void enable_apic();
void disable_apic();

void init_ipi();
void lapic_send_ipi(uint8_t cpu_id, uint8_t vector);

void init_apic();





