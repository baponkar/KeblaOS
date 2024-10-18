#pragma once

#include "../stdlib/stdint.h"
#include "../util/util.h"
#include "../idt/idt.h"
#include "../driver/vga.h"
#include "../driver/ports.h"

void initTimer();
void onIrq0(registers_t *regs);
void delay(uint32_t ms);
