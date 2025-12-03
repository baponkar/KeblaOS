
#include "../driver/io/serial.h"

#include "window.h"



void save_window_background(Window* win) {
    if (win->background_buffer) {
        free(win->background_buffer);
    }
    
    // Allocate buffer for window area (including borders)
    int buffer_size = (win->width + 1) * (win->height + 1) * sizeof(uint32_t);
    win->background_buffer = malloc(buffer_size);
    
    if (win->background_buffer) {
        // Copy the screen content behind the window to buffer
        for (int y = 0; y <= win->height; y++) {
            for (int x = 0; x <= win->width; x++) {
                int screen_x = win->x + x;
                int screen_y = win->y + y;
                
                if (screen_x < SCREEN_WIDTH && screen_y < SCREEN_HEIGHT) {
                    win->background_buffer[y * (win->width + 1) + x] = 
                        get_pixel(screen_x, screen_y);
                }
            }
        }
        win->background_saved = true;
    }
}


void restore_window_background(Window* win) {
    if (!win->background_buffer || !win->background_saved) return;
    
    // Restore the saved background to screen
    for (int y = 0; y <= win->height; y++) {
        for (int x = 0; x <= win->width; x++) {
            int screen_x = win->x + x;
            int screen_y = win->y + y;
            
            if (screen_x < SCREEN_WIDTH && screen_y < SCREEN_HEIGHT) {
                UG_DrawPixel(screen_x, screen_y, 
                           win->background_buffer[y * (win->width + 1) + x]);
            }
        }
    }
    
    win->background_saved = false;
}

static void window_minimize() {
    serial_printf("Minimize clicked!\n");
}

static void window_maximize() {
    serial_printf("Maximize clicked!\n");
}

static void window_close() {
    serial_printf("Close clicked!\n");
}


void draw_window(Window* win) {
    if (!win->visible) return;

    // Window frame
    UG_DrawFrame(win->x, win->y, win->x + win->width, win->y + win->height, C_BLACK);
    UG_FillFrame(win->x+1, win->y+1, win->x + win->width-1, win->y + win->height-1, C_LIGHT_GRAY);

    // Title bar
    UG_FillFrame(win->x, win->y, win->x + win->width, win->y + 25, C_BLUE);
    UG_FontSelect(&FONT_8X12);
    UG_SetForecolor(C_WHITE);
    UG_SetBackcolor(C_BLUE);
    UG_PutString(win->x + 5, win->y + 5, win->title);

    // Control buttons
    draw_button(&win->btn_min, "_");   // minimize
    draw_button(&win->btn_max, "[ ]"); // maximize
    draw_button(&win->btn_close, "X"); // close
}

void window_init(Window* win, int x, int y, int w, int h, const char* title) {
    win->x = x; win->y = y;
    win->width = w; win->height = h;
    win->visible = true;
    win->minimized = false;
    win->maximized = false;
    strncpy(win->title, title, sizeof(win->title)-1);

    // Buttons: place on titlebar
    int btn_w = 25, btn_h = 20;
    win->btn_min.x = x + w - (btn_w*3) - 5;
    win->btn_min.y = y + 2;
    win->btn_min.width = btn_w;
    win->btn_min.height = btn_h;
    win->btn_min.pressed = false;
    win->btn_min.on_click = window_minimize;

    win->btn_max = win->btn_min;
    win->btn_max.x += btn_w + 2;
    win->btn_max.on_click = window_maximize;

    win->btn_close = win->btn_max;
    win->btn_close.x += btn_w + 2;
    win->btn_close.on_click = window_close;

    win->background_buffer = malloc((w+1)*(h+1)*sizeof(uint32_t));
    win->background_saved = false;

    save_window_background(win);
}


void update_window(Window* win) {
    if (!win->visible) return;

    bool changed = false;

    changed |= update_button(&win->btn_min, "_");
    changed |= update_button(&win->btn_max, "[ ]");
    changed |= update_button(&win->btn_close, "X");

    if (changed) {
        draw_window(win); // redraw only if something changed
    }
}





