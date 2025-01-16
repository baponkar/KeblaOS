
#include "window.h"

Window* create_window(int x, int y, int width, int height, uint64_t background_color, const char* title) {
    Window* win = (Window*)kheap_alloc(sizeof(Window));
    if (!win) {
        return NULL;
    }
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->background_color = background_color;
    strncpy(win->title, title, sizeof(win->title));
    
    // Allocate space for the window framebuffer (for drawing inside the window)
    win->framebuffer = kheap_alloc(width * height * sizeof(uint64_t));
    if (!win->framebuffer) {
        kheap_free((uint64_t *)win, sizeof(Window));
        return NULL;
    }
    memset(win->framebuffer, 0, width * height * sizeof(uint64_t)); // Clear the window (black background)

    return win;
}

void draw_window(Window* win) {
    // Draw the window background
    for (int y = 0; y < win->height; y++) {
        for (int x = 0; x < win->width; x++) {
            set_pixel(win->x + x, win->y + y, win->background_color);
        }
    }

    // Draw the window title at the top of the window
    for (int i = 0; i < strlen(win->title); i++) {
        print_char_at(win->y, win->x + i * 8, win->title[i]);
    }
}

void destroy_window(Window* win) {
    if (win) {
        kheap_free((uint64_t *)win->framebuffer,  sizeof(Window));
        kheap_free((uint64_t *)win,  sizeof(Window));
    }
}

#define MAX_WINDOWS 10
Window* window_list[MAX_WINDOWS];
int window_count = 0;

void add_window(Window* win) {
    if (window_count < MAX_WINDOWS) {
        window_list[window_count++] = win;
    }
}

void render_all_windows() {
    for (int i = 0; i < window_count; i++) {
        draw_window(window_list[i]);
    }
}



Mouse current_mouse;

void update_mouse_position(int x, int y, bool clicked) {
    current_mouse.x = x;
    current_mouse.y = y;
    current_mouse.clicked = clicked;
}

Window* get_window_at(int x, int y) {
    for (int i = 0; i < window_count; i++) {
        Window* win = window_list[i];
        if (x >= win->x && x < win->x + win->width &&
            y >= win->y && y < win->y + win->height) {
            return win;
        }
    }
    return NULL;
}

void handle_mouse_input() {
    if (current_mouse.clicked) {
        Window* win = get_window_at(current_mouse.x, current_mouse.y);
        if (win) {
            // Handle window focus or activation
            // For simplicity, let's just bring this window to the front
            // by re-rendering all windows
            render_all_windows();
        }
    }
}






void draw_button(Window* win, int x, int y, int width, int height, const char* label) {
    // Draw button border
    draw_rectangle(win->x + x, win->y + y, width, height, 0xFFFFFF); // White border
    fill_rectangle(win->x + x + 1, win->y + y + 1, width - 2, height - 2, 0xBBBBBB); // Button color

    // Draw label (simplified, you may want to center it)
    for (int i = 0; i < strlen(label); i++) {
        print_char_at(win->y + y + 1, win->x + x + i * 8 + 1, label[i]);
    }
}


Window* focused_window = NULL;

void set_focus_window(Window* win) {
    focused_window = win;
    // Optionally, render the window with a border to indicate focus
}

void bring_window_to_front(Window* win) {
    // Move the window to the last of the list or to the front of the display list
    for (int i = 0; i < window_count; i++) {
        if (window_list[i] == win) {
            // Swap with the last window
            Window* temp = window_list[i];
            window_list[i] = window_list[window_count - 1];
            window_list[window_count - 1] = temp;
            break;
        }
    }
}


