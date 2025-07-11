#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../acpi.h"


struct hpet{
    acpi_header_t header;  // Standard ACPI header
    uint32_t event_timer_block_id;
    GenericAddressStructure_t base_address;
    uint8_t hpet_number;
    uint16_t min_tick;
    uint8_t attributes;
} __attribute__((packed));
typedef struct hpet hpet_t;

void hpet_init(hpet_t* hpet);
void hpet_sleep_us(uint64_t microseconds);
void hpet_enable_periodic_irq(uint8_t irq_number, uint64_t period_fs);


