#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../../../ext_lib/UGUI/ugui.h"
#include "../../../../ext_lib/UGUI/ugui_config.h"

void gui_init();

void ugui_test_1();
void ugui_test_2();

void gui_update();

// Button structure to track state
typedef struct {
    int x, y, width, height;
    bool pressed;
    void (*on_click)();
} Button;


