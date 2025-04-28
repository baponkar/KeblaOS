#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


#include "../int_common.h"

void bsp_apic_int_init();
void ap_apic_int_init(uint64_t core_id);








