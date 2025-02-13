#pragma once

#includee <stdint.h>
#include <stddef.h>
#include <stdbool.h>


struct interrupt_descriptor
{
    uint16_t address_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t address_mid;
    uint32_t address_high;
    uint32_t reserved;
} __attribute__((packed));
typedef struct interrupt_descriptor interrupt_descriptor_t;

struct idtr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));
typedef struct idtr idtr_t;

