#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../../../ext_lib/UGUI/ugui.h"
#include "../../../../ext_lib/UGUI/ugui_config.h"

extern uint64_t SCREEN_WIDTH;
extern uint64_t SCREEN_HEIGHT;
extern uint64_t SCREEN_PITCH;
extern uint64_t SCREEN_BPP;


void gui_init();
