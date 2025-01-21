#pragma once

#include <stdint.h>

typedef struct {
    int x, y;                  // Position on screen
    int width, height;         // Dimensions of the window
    uint64_t background_color; // Background color
    char title[256];           // Title of the window
    uint64_t *framebuffer;     // Area where the window content will be drawn
} Window;

typedef struct {
    int x, y;
    bool clicked;
} Mouse;

Window* create_window(int x, int y, int width, int height, uint64_t background_color, const char* title);
void draw_window(Window* win);
void destroy_window(Window* win);
void add_window(Window* win);
void render_all_windows();
void update_mouse_position(int x, int y, bool clicked);
Window* get_window_at(int x, int y);
void handle_mouse_input();
void set_focus_window(Window* win);
void bring_window_to_front(Window* win);
void draw_button(Window* win, int x, int y, int width, int height, const char* label);
void set_focus_window(Window* win);
void bring_window_to_front(Window* win);


