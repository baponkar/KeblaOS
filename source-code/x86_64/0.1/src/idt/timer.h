#pragma once

#include <stdint.h>
#include "../util/util.h"
#include "../idt/idt.h"
#include "../driver/vga.h"
#include "../driver/ports.h"

void initTimer();
void timerHandler(registers_t *regs);
void delay(uint32_t ms);
