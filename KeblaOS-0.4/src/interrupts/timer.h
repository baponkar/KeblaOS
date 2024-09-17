#pragma once

#include "../stdlib/stdint.h"

void initTimer();
void onIrq0(struct InterruptRegisters *regs);
void delay(uint32_t ms);