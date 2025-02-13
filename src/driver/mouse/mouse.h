#pragma once

#include <stdint.h>





void mouse_wait(uint8_t type);
void mouse_write(uint8_t data);
uint8_t mouse_read();
void mouse_install();
void draw_mouse_cursor();
void erase_mouse_cursor(int old_x, int old_y);
void mouse_handler();
void pit_handler();
void initialize_drivers();
void main_loop();
