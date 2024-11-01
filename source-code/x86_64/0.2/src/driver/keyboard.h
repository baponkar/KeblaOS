#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "../idt/idt.h"
#include "../util/util.h"
#include "../driver/ports.h"

#define KEYBOARD_COMMAND_PORT 0x64  // Keyboard Command Port
#define KEYBOARD_DATA_PORT 0x60     // Keyboard Data Port

void init_keyboard();

