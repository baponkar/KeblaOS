#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../acpi/descriptor_table/hpet.h"


void hpet_init();
void hpet_sleep(uint32_t ms);