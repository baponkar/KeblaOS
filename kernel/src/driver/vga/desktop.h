#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../../../ext_lib/UGUI/ugui.h"
#include "../../../../ext_lib/UGUI/ugui_config.h"

#include "ugui_wrapper.h"



void draw_button(Button* btn, const char* text);
bool update_button(Button* btn, const char* text); 

void desktop_init();

void draw_window(Window* win);
void window_init(Window* win, int x, int y, int w, int h, const char* title);
void update_window(Window* win);








