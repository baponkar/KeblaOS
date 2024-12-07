#pragma once

#include "../stdlib/stdint.h"
#include "../driver/vga.h"

struct gdt_entry_struct
{//64 Bit
    uint64_t    limit_low   :16;    // limit = 16+4 = 20
    uint64_t    base_low    :16;    // base = 16+8+8 = 32
    uint64_t    base_mid    :8;
    uint64_t    access      :8;     // access = 8
    uint64_t    limit_up    :4;
    uint64_t    flags       :4;     // flags = 4
    uint64_t    base_up     :8;
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;


