#pragma once

#include "idt.h"

void initTimer();
void onIrq0(registers_t *regs);
void delay(uint32_t ms);