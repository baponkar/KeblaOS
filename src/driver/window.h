#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../lib/stdlib.h"
#include "../lib/stdio.h"

#include "../x86_64/pit/pit_timer.h"

#include "vga.h"

#define MAX_WINDOWS 10


typedef struct {
    int x, y, width, height;
    uint64_t color;
    char text[16];
    void (*on_click)();
} button_t;


typedef struct {
    int x, y;
    int width, height;
    uint64_t border_color;
    uint64_t background_color;
    bool active;
    char title[32];
    button_t buttons[5]; // Maximum of 5 buttons per window
    size_t button_count;
} window_t;

typedef enum { KEY_PRESS, MOUSE_CLICK, WINDOW_RESIZE } event_type_t;
typedef struct {
    event_type_t type;
    int x, y;
    char keycode;
} event_t;


extern window_t windows[MAX_WINDOWS];

void push_event(event_t event);
event_t pop_event();

void create_window(int x, int y, int width, int height, const char* title);
void draw_window(window_t* window);
void move_window(window_t* window, int new_x, int new_y);
void close_window(window_t* window);
void render_gui();

void setup_gui();

void draw_cursor(int x, int y);
void move_cursor(int new_x, int new_y);

void draw_button(button_t* button);
void add_button_to_window(window_t* window, int x, int y, int width, int height, const char* text, void (*on_click)());

void on_ok_click();
void on_cancel_click();


void show_loading_animation_graphic(int x, int y, int width, uint64_t border_color, uint64_t fill_color, int duration_ms);

