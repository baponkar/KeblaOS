#pragma once

#include <stdint.h>

#include "../../bootloader/acpi.h"


struct HpetSdt {
    acpi_header_t header;
    uint32_t event_timer_block_id;
    uint32_t reserved;
    uint64_t address;
    uint8_t id;
    uint16_t min_ticks;
    uint8_t page_protection;
}__attribute__((packed));

