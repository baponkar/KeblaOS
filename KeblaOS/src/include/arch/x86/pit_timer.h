#pragma once

#include <stdint.h>

#include "../../util/util.h"

void init_timer(uint16_t interval);
void timerHandler(registers_t *regs);
void delay(uint32_t ms);
