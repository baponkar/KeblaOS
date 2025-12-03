


#include "../driver/io/serial.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/math.h"
#include "../lib/time.h"

#include "../sys/acpi/descriptor_table/fadt.h"

#include "../driver/vga/framebuffer.h"
#include "../driver/vga/vga.h"
#include "../driver/vga/data/image_data.h"
#include "../driver/vga/data/icon_data.h"

#include "../driver/mouse/mouse.h"

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


static void window_minimize() {
    printf("Minimize clicked!\n");
    my_window.visible = false;
    restore_window_background(&my_window);
}

static void window_maximize() {
    printf("Maximize clicked!\n");
    my_window.x = 0;
    my_window.y = 0;
    my_window.width = SCREEN_WIDTH;
    my_window.height = SCREEN_HEIGHT - TASKBAR_HEIGHT;
    draw_window(&my_window);
}

static void window_close() {
   printf("Close clicked!\n");
}

static void start_btn_callback(){
    serial_printf("Start Button Clicked!\n");
    window_init(&my_window, 50, 50, 600, 400, "My First Window");
    draw_window(&my_window);

    my_window.btn_min.on_click = window_minimize;
    my_window.btn_max.on_click = window_maximize;
    my_window.btn_close.on_click = window_close;
}

static void poweroff_btn_callback() {
    serial_printf("Poweroff clicked!\n");
    acpi_poweroff();
}

static void reboot_btn_callback() {
    serial_printf("Reboot clicked!\n");
    acpi_reboot();
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













