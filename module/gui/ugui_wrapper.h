#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#include "../ext_lib/UGUI/ugui.h"
#include "../ext_lib/UGUI/ugui_config.h"


uint64_t SCREEN_WIDTH;
uint64_t SCREEN_HEIGHT;
uint64_t SCREEN_PITCH;
uint64_t SCREEN_BPP;

// Button structure to track state
typedef struct {
    int x, y, width, height;
    bool hovered;
    bool pressed;
    bool prev_left_pressed;
    void (*on_click)();
} Button;

typedef struct {
    int x, y;               // Top-left position
    int width, height;      // Size
    bool visible;           // Shown or hidden
    bool minimized;
    bool maximized;

    char title[64];         // Window title

    // Control buttons
    Button btn_min;
    Button btn_max;
    Button btn_close;
} Window;


void gui_init();


