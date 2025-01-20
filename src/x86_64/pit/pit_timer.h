#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../util/util.h"
#include "../idt/idt.h"
#include "../../driver/vga.h"
#include "../../driver/ports.h"


void init_timer(uint16_t interval);
void timerHandler(registers_t *regs);
void delay(uint32_t ms);


