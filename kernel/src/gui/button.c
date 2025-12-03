
#include "button.h"




void save_button_background(Button* btn) {
    if (btn->background_buffer) {
        free(btn->background_buffer);
    }
    
    // Allocate buffer for window area (including borders)
    int buffer_size = (btn->width + 1) * (btn->height + 1) * sizeof(uint32_t);
    btn->background_buffer = malloc(buffer_size);
    
    if (btn->background_buffer) {
        // Copy the screen content behind the window to buffer
        for (int y = 0; y <= btn->height; y++) {
            for (int x = 0; x <= btn->width; x++) {
                int screen_x = btn->x + x;
                int screen_y = btn->y + y;
                
                if (screen_x < SCREEN_WIDTH && screen_y < SCREEN_HEIGHT) {
                    btn->background_buffer[y * (btn->width + 1) + x] = 
                        get_pixel(screen_x, screen_y);
                }
            }
        }
        btn->background_saved = true;
    }
}


void restore_button_background(Button* btn) {
    if (!btn->background_buffer || !btn->background_saved) return;
    
    // Restore the saved background to screen
    for (int y = 0; y <= btn->height; y++) {
        for (int x = 0; x <= btn->width; x++) {
            int screen_x = btn->x + x;
            int screen_y = btn->y + y;
            
            if (screen_x < SCREEN_WIDTH && screen_y < SCREEN_HEIGHT) {
                UG_DrawPixel(screen_x, screen_y, 
                           btn->background_buffer[y * (btn->width + 1) + x]);
            }
        }
    }
    
    btn->background_saved = false;
}

void draw_button(Button* btn, const char* text) {
    btn->hide = false;

    btn->background_buffer = malloc((btn->width+1)*(btn->height+1)*sizeof(uint32_t));
    btn->background_saved = false;
    uint32_t bg_color = C_RED;

    if (btn->pressed) {
        bg_color = C_DARK_GRAY;     // Pressed color
    } else if (btn->hovered) {
        bg_color = C_LIGHT_CORAL;   // Hover color
    }

    // Border drawing (raised vs pressed)
    if (btn->pressed) {
        UG_DrawLine(btn->x, btn->y, btn->x + btn->width, btn->y, C_DARK_GRAY);
        UG_DrawLine(btn->x, btn->y, btn->x, btn->y + btn->height, C_DARK_GRAY);
        UG_DrawLine(btn->x, btn->y + btn->height, btn->x + btn->width, btn->y + btn->height, C_WHITE);
        UG_DrawLine(btn->x + btn->width, btn->y, btn->x + btn->width, btn->y + btn->height, C_WHITE);
    } else {
        UG_DrawLine(btn->x, btn->y, btn->x + btn->width, btn->y, C_WHITE);
        UG_DrawLine(btn->x, btn->y, btn->x, btn->y + btn->height, C_WHITE);
        UG_DrawLine(btn->x, btn->y + btn->height, btn->x + btn->width, btn->y + btn->height, C_DARK_GRAY);
        UG_DrawLine(btn->x + btn->width, btn->y, btn->x + btn->width, btn->y + btn->height, C_DARK_GRAY);
    }

    // Fill background
    UG_FillFrame(btn->x+1, btn->y+1, btn->x+btn->width-1, btn->y+btn->height-1, bg_color);

    // Draw text centered
    UG_FontSelect(&FONT_8X12);
    UG_SetForecolor(C_WHITE);
    UG_SetBackcolor(bg_color);
    int text_x = btn->x + (btn->width / 2) - ((strlen(text) * 8) / 2);
    int text_y = btn->y + (btn->height / 2) - (12 / 2);
    UG_PutString(text_x, text_y, text);
}

void hide_button(Button* btn) {
    btn->hide = true;
    UG_FillFrame(btn->x, btn->y, btn->x + btn->width, btn->y + btn->height, C_BLACK);
}


bool update_button(Button *btn, const char *text) {
    bool changed = false;

    int mx = get_mouse_x();
    int my = get_mouse_y();
    bool left = is_left_button_pressed();

    bool inside = (mx >= btn->x && mx < btn->x + btn->width &&
                   my >= btn->y && my < btn->y + btn->height);

    // Hover state
    if (inside != btn->hovered) {
        btn->hovered = inside;
        changed = true;
    }

    // Press start
    if (inside && left && !btn->prev_left_pressed) {
        btn->pressed = true;
        changed = true;
    }

    // Mouse released: if it was pressed and still inside, that's a click
    if (!left && btn->prev_left_pressed && btn->pressed) {
        btn->pressed = false;
        changed = true;

        if (inside && btn->on_click) {
            btn->on_click();   
        }
    }

    btn->prev_left_pressed = left;

    if (changed) {
        draw_button(btn, text);   
    }

    return changed;
}



