#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "gdt.h"
#include "tss.h"

#define TOTAL_GDT_ENTRIES 7                // 5 GDT(64 Bit) + 1 TSS (128 Bit)



void init_gdt_tss_in_cpu(size_t cpu_id);






