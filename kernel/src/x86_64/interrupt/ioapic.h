#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void ioapic_write_reg(uint32_t reg, uint32_t value);
uint32_t ioapic_read_reg(uint32_t reg);

void enable_ioapic_mode();

void ioapic_route_irq(uint8_t irq, uint8_t apic_id, uint8_t vector, uint32_t flags);

void ioapic_init();

void configure_mouse_irq();
void configure_keyboard_irq();
