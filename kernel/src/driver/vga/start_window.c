

#include "../../../../ext_lib/UGUI/ugui.h"
#include "../../../../ext_lib/UGUI/ugui_config.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/math.h"
#include "../../lib/time.h"

#include "framebuffer.h"
#include "vga.h"
#include "data/image_data.h"
#include "data/icon_data.h"

#include "../mouse/mouse.h"

#include "ugui_wrapper.h"

#include "start_window.h"




// Taskbar
#define TASKBAR_HEIGHT 40

// Start Button
#define START_BUTTON_WIDTH 100
#define START_BUTTON_HEIGHT 40


UG_GUI gui_main;

void draw_taskbar() {
    // Draw taskbar background
    UG_FillFrame(0, SCREEN_HEIGHT - TASKBAR_HEIGHT, 
                 SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, C_DARK_GRAY);
    
    // Draw top border
    UG_DrawLine(0, SCREEN_HEIGHT - TASKBAR_HEIGHT, 
                SCREEN_WIDTH - 1, SCREEN_HEIGHT - TASKBAR_HEIGHT, C_WHITE);
}


// Start button
Button start_button;


void start_button_callback() {
    printf("Start Button clicked!\n");
}


void draw_start_button(Button* btn) {
    if (btn->pressed) {
        // Pressed state
        UG_DrawFrame(btn->x, btn->y, btn->x + btn->width, btn->y + btn->height, C_WHITE);
        UG_FillFrame(btn->x + 2, btn->y + 2, btn->x + btn->width - 2, btn->y + btn->height - 2, C_BLUE);
    } else {
        // Normal state
        UG_DrawFrame(btn->x, btn->y, btn->x + btn->width, btn->y + btn->height, C_WHITE);
        UG_FillFrame(btn->x + 1, btn->y + 1, btn->x + btn->width - 1, btn->y + btn->height - 1, C_BLUE);
    }
    
    // Draw start text
    UG_FontSelect(&FONT_12X16);
    UG_SetForecolor(C_WHITE);
    UG_SetBackcolor(C_BLUE);
    UG_PutString(btn->x + 15, btn->y + 12, "Start");
}



void update_start_button(Button* btn) {
    static bool prev_left_pressed = false;
    bool current_left_pressed = is_left_button_pressed();
    
    bool was_pressed = btn->pressed;
    btn->pressed = mouse_in_rect(btn->x, btn->y, btn->width, btn->height) && current_left_pressed;
    
    // If button state changed, redraw it
    if (was_pressed != btn->pressed) {
        draw_start_button(btn);
    }
    
    // Check for click (button was pressed and now is released)
    if (prev_left_pressed && !current_left_pressed && mouse_in_rect(btn->x, btn->y, btn->width, btn->height)) {
        if (btn->on_click) btn->on_click();
    }
    
    prev_left_pressed = current_left_pressed;
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


void draw_desktop() {

    // Fill background with a nice color (Windows-like blue)
    UG_FillScreen(C_LIGHT_BLUE);

    // Drawing Icon Image
    extern const uint64_t kebla_boy[];
    draw_image_with_transparency(180, 100,  kebla_boy, KEBLA_BOY_WIDTH, KEBLA_BOY_HEIGHT);

    extern const uint64_t delete[];
    draw_image_with_transparency(50, 75,  delete, MONITOR_WIDTH, MONITOR_HEIGHT);

    
    // Add monitor icons using placeholders
    // UG_FillFrame(50, 50, 90, 90, C_WHITE);
    // UG_DrawFrame(50, 50, 90, 90, C_BLACK);
    
    // Draw My Computer icon using an image
    extern const uint64_t monitor[];
    // display_image(50, 50, monitor, MONITOR_WIDTH, MONITOR_HEIGHT);
    draw_image_with_transparency(50, 160,  monitor, MONITOR_WIDTH, MONITOR_HEIGHT);


    // Add documentation icon using placeholder
    // UG_FillFrame(150, 50, 190, 90, C_WHITE);
    // UG_DrawFrame(150, 50, 190, 90, C_BLACK);
    
    // Draw Documents icon using an image
    extern const uint64_t documentation[];
    draw_image_with_transparency(50, 220, documentation, DOCUMENTATION_HEIGHT, DOCUMENTATION_WIDTH);
    
    // Add text labels below the icons
    UG_FontSelect(&FONT_8X12);
    UG_SetForecolor(C_BLACK);
    UG_SetBackcolor(C_LIGHT_BLUE);
    UG_PutString(50, 130, "Trash");
    UG_PutString(50, 190, "My Computer");
    UG_PutString(50, 290, "Documents");
}




void windows_like_ui() {
    gui_init();

    // Initialize start button
    start_button.x = 0;
    start_button.y = SCREEN_HEIGHT - TASKBAR_HEIGHT;
    start_button.width = START_BUTTON_WIDTH;
    start_button.height = START_BUTTON_HEIGHT;
    start_button.pressed = false;
    start_button.on_click = start_button_callback;

    // Draw initial UI
    draw_desktop();
    draw_taskbar();
    draw_start_button(&start_button);

    // Initial clock display
    update_clock_display();

    // Main loop
    while (1) {
        update_start_button(&start_button);
        
        // Update clock periodically
        static uint64_t last_time = 0;
        uint64_t current_time = get_uptime_seconds(0) * 1000000; // Convert to microseconds
        
        if (current_time - last_time >= CLOCK_UPDATE_INTERVAL) {
            update_clock_display();
            last_time = current_time;
        }
        
        UG_Update();
        usleep(0, 5000); // 50ms delay (20 FPS)
    }

}

