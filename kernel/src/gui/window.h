#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/time.h"

#include "../driver/vga/color.h"
#include "../driver/vga/ugui_wrapper.h"
#include "../driver/vga/vga.h"
#include "../driver/vga/framebuffer.h"

#include "../../../ext_lib/UGUI/ugui.h"
#include "../../../ext_lib/UGUI/ugui_config.h"
#include "../driver/vga/ugui_wrapper.h"

#include "../driver/mouse/mouse.h"

#include "button.h"




typedef struct Window{
    int x, y;               // Top-left position
    int width, height;      // Size

    bool visible;           // Shown or hidden
    bool minimized;
    bool maximized;

    char title[64];         // Window title

    uint32_t* background_buffer;
    bool background_saved;

    // Control buttons
    Button btn_min;
    Button btn_max;
    Button btn_close;
} Window;


void save_window_background(Window* win);
void restore_window_background(Window* win);


void draw_window(Window* win);
void window_init(Window* win, int x, int y, int w, int h, const char* title);
void update_window(Window* win);





