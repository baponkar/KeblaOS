#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>




void mmio_write(uint32_t address, uint32_t value); // 
uint32_t mmio_read(uint32_t address);

int has_apic();
uint32_t get_lapic_id();

void lapic_send_ipi(uint8_t cpu_id, uint8_t vector);



void enable_apic();
void apic_send_eoi();

void enable_ioapic_mode();

void ioapic_route_irq(uint8_t irq, uint8_t apic_id, uint8_t vector);

void init_apic_interrupt();



