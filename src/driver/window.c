
#include "window.h"

window_t windows[MAX_WINDOWS];
size_t window_count = 0;
int cursor_x = 0, cursor_y = 0;

void create_window(int x, int y, int width, int height, const char* title) {
    if (window_count >= MAX_WINDOWS) return;
    window_t* win = &windows[window_count++];
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->border_color = COLOR_WHITE;
    win->background_color = COLOR_GRAY;
    win->active = true;
    strncpy(win->title, title, sizeof(win->title) - 1);
    draw_rectangle(x, y, width, height, win->border_color);
    fill_rectangle(x + 1, y + 1, width - 2, height - 2, win->background_color);
    print_char_at(y + 2, x + 2, '[');
    print(win->title);
    print_char_at(y + 2, x + 2 + strlen(win->title) + 1, ']');
}

void draw_window(window_t* win) {
    draw_rectangle(win->x, win->y, win->width, win->height, win->border_color);
    fill_rectangle(win->x + 1, win->y + 1, win->width - 2, win->height - 2, win->background_color);
    print_char_at(win->y + 2, win->x + 2, '[');
    print(win->title);
    print_char_at(win->y + 2, win->x + 2 + strlen(win->title) + 1, ']');
}

void move_window(window_t* win, int new_x, int new_y) {
    win->x = new_x;
    win->y = new_y;
    draw_window(win);
}

void close_window(window_t* win) {
    fill_rectangle(win->x, win->y, win->width, win->height, COLOR_BLACK);
    *win = windows[--window_count];
}

void draw_cursor(int x, int y) {
    set_pixel(x, y, COLOR_WHITE);
}

void move_cursor(int new_x, int new_y) {
    set_pixel(cursor_x, cursor_y, COLOR_BLACK);
    cursor_x = new_x;
    cursor_y = new_y;
    draw_cursor(cursor_x, cursor_y);
}

void render_gui() {
    clear_screen();
    for (size_t i = 0; i < window_count; i++) {
        draw_window(&windows[i]);
    }
    draw_cursor(cursor_x, cursor_y);
}

void add_button_to_window(window_t* window, int x, int y, int width, int height, const char* text, void (*on_click)()) {
    if (window->button_count >= 5) return; // Max button limit reached

    button_t* button = &window->buttons[window->button_count++];
    button->x = window->x + x;
    button->y = window->y + y;
    button->width = width;
    button->height = height;
    button->color = COLOR_GRAY; // Light Gray
    strncpy(button->text, text, sizeof(button->text) - 1);
    button->on_click = on_click;
}


void on_ok_click() {
    print("OK button clicked!\n");
}

void on_cancel_click() {
    print("Cancel button clicked!\n");
}

void setup_gui() {
    create_window(10, 10, 100, 50, "Main Menu");
    add_button_to_window(&windows[0], 10, 20, 30, 10, "OK", on_ok_click);
    add_button_to_window(&windows[0], 50, 20, 30, 10, "Cancel", on_cancel_click);
    draw_window(&windows[0]);
}

void show_loading_animation_graphic(int x, int y, int width, uint64_t border_color, uint64_t fill_color, int duration_ms) {
    int bar_length = width; // Total width of the loading bar
    int interval_us = 1; // Update interval in microseconds (50ms per step)
    int total_steps = (duration_ms * 1) / interval_us;

    // Draw the border of the loading bar
    draw_rectangle(x, y, bar_length, 10, border_color);

    // Animate the loading bar
    for (int i = 0; i <= total_steps; i++) {
        int fill_width = (i * bar_length) / total_steps; // Calculate the filled width
        fill_rectangle(x + 1, y + 1, fill_width - 2, 8, fill_color); // Fill the bar
        delay(interval_us); // Wait for the next frame
    }

    // Completion message
    print_char_at(y + 2, x + bar_length / 2 - 4, '[');
    print("Done");
    print_char_at(y + 2, x + bar_length / 2 + 4, ']');
}


