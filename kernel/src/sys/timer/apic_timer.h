#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

uint8_t get_core_id();
void calibrate_apic_timer_pit();
void calibrate_apic_timer_tsc();

void init_apic_timer(uint32_t interval_ms);
void apic_delay(uint32_t milliseconds);

