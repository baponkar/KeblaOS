#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


void init_apic_timer(uint32_t interval_ms);
void apic_delay(uint32_t milliseconds);

