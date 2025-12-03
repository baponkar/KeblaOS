#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/time.h"

#include "../driver/vga/color.h"
#include "../driver/vga/ugui_wrapper.h"
#include "../driver/vga/vga.h"
#include "../driver/vga/framebuffer.h"

#include "../../../ext_lib/UGUI/ugui.h"
#include "../../../ext_lib/UGUI/ugui_config.h"
#include "../driver/vga/ugui_wrapper.h"

#include "../driver/mouse/mouse.h"


typedef struct {
    int x, y, width, height;
    bool hovered;
    bool pressed;
    bool prev_left_pressed;

    uint32_t background_color;
    uint32_t hover_color;
    uint32_t pressed_color;
    uint32_t border_color;

    bool hide;
    uint32_t* background_buffer;
    bool background_saved;

    void (*on_click)();
} Button;


void draw_button(Button* btn, const char* text);
bool update_button(Button* btn, const char* text);









