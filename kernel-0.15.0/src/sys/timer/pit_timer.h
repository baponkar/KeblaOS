#pragma once

#include <stdint.h>


void enable_pit_timer();
void disable_pit_timer();

void init_pit_timer();
void pit_sleep(uint32_t ms);

