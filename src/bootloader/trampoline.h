#pragma once

#include <stdint.h>


void load_trampoline_for_ap(uint64_t core_id);
void wakeup_ap(uint64_t apic_id);
