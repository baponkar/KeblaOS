


#include "../../driver/io/serial.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/math.h"
#include "../../lib/time.h"

#include "../../sys/acpi/descriptor_table/fadt.h"

#include "framebuffer.h"
#include "vga.h"
#include "data/image_data.h"
#include "data/icon_data.h"

#include "../mouse/mouse.h"

#include "window.h"

#include "desktop.h"



#define TASKBAR_HEIGHT 40
#define TASKBAR_WIDTH SCREEN_WIDTH


#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 40

Button start_button;
Button poweroff_button;
Button reboot_button;

UG_GUI gui_main;

static Window my_window;

static void start_btn_callback(){
    serial_printf("Start Button Clicked!\n");
    // void window_init(Window* win, int x, int y, int w, int h, const char* title)
    window_init(&my_window, 50, 50, 600, 400, "My First Window");
    draw_window(&my_window);
}

static void poweroff_btn_callback() {
    serial_printf("Poweroff clicked!\n");
    // TODO: call your kernel poweroff function (e.g. acpi_poweroff())
    acpi_poweroff();
}

static void reboot_btn_callback() {
    serial_printf("Reboot clicked!\n");
    // TODO: call your kernel reboot function (e.g. cpu_reboot())
    acpi_reboot();
}


void draw_button(Button* btn, const char* text) {
    uint32_t bg_color = C_RED;

    if (btn->pressed) {
        bg_color = C_DARK_GRAY;     // Pressed color
    } else if (btn->hovered) {
        bg_color = C_LIGHT_CORAL;  // Hover color
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



bool update_button(Button* btn, const char* text) {
    bool current_left_pressed = is_left_button_pressed();
    bool changed = false;

    // Hover detection
    bool was_hovered = btn->hovered;
    btn->hovered = mouse_in_rect(btn->x, btn->y, btn->width, btn->height);
    if (btn->hovered != was_hovered) {
        changed = true;
    }

    // Press detection
    bool was_pressed = btn->pressed;
    btn->pressed = btn->hovered && current_left_pressed;
    if (btn->pressed != was_pressed) {
        changed = true;
    }

    // Redraw if hover/pressed state changed
    if (changed) {
        draw_button(btn, text);
    }

    // On release → fire callback
    if (btn->prev_left_pressed && !current_left_pressed && btn->hovered) {
        if (btn->on_click) btn->on_click();
        changed = true; // state changed due to click action
    }

    btn->prev_left_pressed = current_left_pressed;
    return changed;
}




static void draw_taskbar() {
    // Draw taskbar background
    UG_FillFrame(0, SCREEN_HEIGHT - TASKBAR_HEIGHT, 
                 SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, C_DARK_GRAY);
    
    // Draw top border
    UG_DrawLine(0, SCREEN_HEIGHT - TASKBAR_HEIGHT, 
                SCREEN_WIDTH - 1, SCREEN_HEIGHT - TASKBAR_HEIGHT, C_WHITE);
}


// Add these global variables
static char clock_str[9] = "00:00:00"; // HH:MM:SS format
static uint64_t last_clock_update = 0;
static const uint64_t CLOCK_UPDATE_INTERVAL = 1000000; // Update every second (1,000,000 microseconds)

// Function to update the clock display
void update_clock_display() {
    // Get current time
    time_t current_time = time(NULL);
    struct tm *time_info = localtime(&current_time);
    
    // Format time as HH:MM:SS
    snprintf(clock_str, sizeof(clock_str), "%d:%d:%d", 
             time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
    
    // Calculate position for clock (right side of taskbar)
    int clock_x = SCREEN_WIDTH - 80;
    int clock_y = SCREEN_HEIGHT - TASKBAR_HEIGHT + 12;
    
    // Clear previous clock area
    UG_FillFrame(clock_x, SCREEN_HEIGHT - TASKBAR_HEIGHT + 1, 
                 SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, C_DARK_GRAY);
    
    // Draw new clock
    UG_FontSelect(&FONT_8X12);
    UG_SetForecolor(C_WHITE);
    UG_SetBackcolor(C_DARK_GRAY);
    UG_PutString(clock_x - 40, clock_y, "GMT ");
    UG_PutString(clock_x, clock_y, clock_str);
}


static void draw_desktop() {

    // Fill background with a nice color (Windows-like blue)
    UG_FillScreen(C_LIGHT_BLUE);

    draw_taskbar();

    // Drawing Desktop Wallapaper Image
    extern const uint64_t kebla_boy[];
    draw_image_with_transparency(180, 100,  kebla_boy, KEBLA_BOY_WIDTH, KEBLA_BOY_HEIGHT);

    // Add text labels below the icons
    UG_FontSelect(&FONT_8X12);
    UG_SetForecolor(C_BLACK);
    UG_SetBackcolor(C_LIGHT_BLUE);

    // Draw Trash icon 
    extern const uint64_t delete[];
    draw_image_with_transparency(50, 80,  delete, MONITOR_WIDTH, MONITOR_HEIGHT);
    UG_PutString(50, 140, "Trash");

    
    // Draw My Computer icon using an image
    extern const uint64_t monitor[];
    draw_image_with_transparency(50, 160,  monitor, MONITOR_WIDTH, MONITOR_HEIGHT);
    UG_PutString(50, 220, "My Computer");

    
    // Draw Documents icon using an image
    extern const uint64_t documentation[];
    draw_image_with_transparency(50, 240, documentation, DOCUMENTATION_HEIGHT, DOCUMENTATION_WIDTH);
    UG_PutString(50, 300, "Documents");

    // Draw Info icon
    extern const uint64_t info[];
    draw_image_with_transparency(50, 320, info, INFO_HEIGHT, INFO_WIDTH);
    UG_PutString(50, 380, "Info");
    
}



void desktop_init(){

    gui_init();
    // UG_SelectGUI(&gui_main);

    draw_desktop();

    // Initialize start button
    start_button.x = 0;
    start_button.y = SCREEN_HEIGHT - TASKBAR_HEIGHT;
    start_button.width = BUTTON_WIDTH;
    start_button.height = BUTTON_HEIGHT;
    start_button.pressed = false;
    start_button.on_click = start_btn_callback;

    draw_button((Button *) &start_button, "Start");

    // Poweroff button (next to Start)
    poweroff_button.x = BUTTON_WIDTH + 10;
    poweroff_button.y = SCREEN_HEIGHT - TASKBAR_HEIGHT;
    poweroff_button.width = BUTTON_WIDTH;
    poweroff_button.height = BUTTON_HEIGHT;
    poweroff_button.pressed = false;
    poweroff_button.on_click = poweroff_btn_callback;

    draw_button((Button *)&poweroff_button, "Poweroff");

    // Reboot button
    reboot_button.x = BUTTON_WIDTH*2 + 20;
    reboot_button.y = SCREEN_HEIGHT - TASKBAR_HEIGHT;
    reboot_button.width = BUTTON_WIDTH;
    reboot_button.height = BUTTON_HEIGHT;
    reboot_button.pressed = false;
    reboot_button.on_click = reboot_btn_callback;

    draw_button((Button *)&reboot_button, "Reboot");
    

    // Main loop
    while (1) {
        if (my_window.visible) {
            update_window(&my_window);
        }

        update_button(&start_button, "Start");
        update_button(&poweroff_button, "Poweroff");
        update_button(&reboot_button, "Reboot");
        
        // Update clock periodically
        static uint64_t last_time = 0;
        uint64_t current_time = get_uptime_seconds(0) * 1000000; // Convert to microseconds
        
        if (current_time - last_time >= CLOCK_UPDATE_INTERVAL) {
            update_clock_display();
            last_time = current_time;
        }
        
        UG_Update();
        usleep(0, 500);    // 50ms delay (20 FPS)
    }
}



// ================================================================

static void window_minimize() {
    serial_printf("Minimize clicked!\n");
    // Example: hide window content
    // win->minimized = true;
}

static void window_maximize() {
    serial_printf("Maximize clicked!\n");
    // Example: set fullscreen
    // win->maximized = true;
}

static void window_close() {
    serial_printf("Close clicked!\n");
    // Example: destroy window
    // win->visible = false;
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


