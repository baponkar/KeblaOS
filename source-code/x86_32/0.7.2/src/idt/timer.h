#pragma once

#include "../stdlib/stdint.h"
#include "../util/util.h"
#include "../idt/idt.h"
#include "../driver/vga.h"
#include "../driver/ports.h"
#include "../driver/rtc.h"
#include "../driver/pcspeaker.h"

#define PIT_CHANNEL_0 0x40          // Programmable Interval Timer
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42          // Generate Sound
#define PIT_COMMAND_PORT 0x43

void initTimer();
void onIrq0(registers_t *regs);
void delay(uint32_t ms);
