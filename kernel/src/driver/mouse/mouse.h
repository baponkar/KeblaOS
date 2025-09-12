#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void enable_mouse();
void disable_mouse();
void mouse_init();

int get_mouse_x();
int get_mouse_y();

bool is_left_button_pressed();
bool is_right_button_pressed();
bool is_middle_button_pressed();
bool mouse_in_rect(int x, int y, int width, int height);

