
#include "../../../../ext_lib/UGUI/ugui.h"
#include "../../../../ext_lib/UGUI/ugui_config.h"


#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/math.h"
#include "../../lib/time.h"

#include "framebuffer.h"
#include "vga.h"

#include "../mouse/mouse.h"

#include "ugui_wrapper.h"

#include "ugui_test.h"



Button test_button;



void button_callback() {
    printf("Button clicked!\n");
    // Add your button action here
}

void draw_button(Button* btn) {
    if (btn->pressed) {
        UG_DrawFrame(btn->x, btn->y, btn->x + btn->width, btn->y + btn->height, C_WHITE);
        UG_FillFrame(btn->x + 2, btn->y + 2, btn->x + btn->width - 2, btn->y + btn->height - 2, C_BLUE);
    } else {
        UG_DrawFrame(btn->x, btn->y, btn->x + btn->width, btn->y + btn->height, C_WHITE);
        UG_FillFrame(btn->x + 1, btn->y + 1, btn->x + btn->width - 1, btn->y + btn->height - 1, C_BLUE);
    }
    
    UG_FontSelect(&FONT_12X16);
    UG_SetForecolor(C_WHITE);
    UG_SetBackcolor(C_BLUE);
    UG_PutString(btn->x + 10, btn->y + 10, "Click Me!");
}

void update_button(Button* btn) {
    bool was_pressed = btn->pressed;
    btn->pressed = mouse_in_rect(btn->x, btn->y, btn->width, btn->height) && is_left_button_pressed();
    
    // If button state changed, redraw it
    if (was_pressed != btn->pressed) {
        draw_button(btn);
    }
    
    // Check for click (button was pressed and now is released)
    static bool prev_left_pressed = false;
    bool current_left_pressed = is_left_button_pressed();
    
    if (prev_left_pressed && !current_left_pressed && 
        mouse_in_rect(btn->x, btn->y, btn->width, btn->height)) {
        if (btn->on_click) btn->on_click();
    }
    
    prev_left_pressed = current_left_pressed;
}


// Main loop function (call this regularly)
static void gui_update() {
    update_button(&test_button);
    UG_Update();
}


void ugui_test_1(){
    gui_init();

    UG_FillScreen(C_BLACK); // Clear screen

    UG_DrawFrame(50, 50, 550, 550, C_WHITE);   // rectangle
    UG_FillFrame(60, 60, 540, 540, C_BLUE);    // filled rectangle

    
    // Initialize button
    test_button.x = 100;
    test_button.y = 100;
    test_button.width = 120;
    test_button.height = 40;
    test_button.pressed = false;
    test_button.on_click = button_callback;

    // Draw the button
    draw_button(&test_button);

    UG_FontSelect(&FONT_12X16);
    UG_SetForecolor(C_WHITE);
    UG_SetBackcolor(C_BLACK);


    UG_PutString(70, 80, "Hello, uGUI!");      // text

    while (1) {
        // Your other main loop code here
        
        // Update GUI and check for button clicks
        gui_update();
        
        // Add a small delay to prevent excessive CPU usage
        // You might need to implement a sleep function based on your OS
        // sleep_ms(16); // ~60 FPS
        usleep(0, 100);
        
    }
}










