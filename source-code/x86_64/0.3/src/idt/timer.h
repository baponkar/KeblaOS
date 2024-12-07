#pragma once

#include <stdint.h>
#include "../util/util.h"
#include "../idt/idt.h"
#include "../driver/vga/vga.h"
#include "../driver/ports.h"

extern uint64_t ticks;
extern uint64_t seconds;

void init_timer();
void timerHandler(registers_t *regs);
void delay(uint64_t ms);
