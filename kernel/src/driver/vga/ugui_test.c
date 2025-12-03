

#include "ugui_wrapper.h"
#include "../gui/button.h"
#include "../gui/window.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/math.h"
#include "../../lib/time.h"

#include "../mouse/mouse.h"

#include "ugui_test.h"



Button test_button;



void button_callback() {
    printf("Button clicked!\n");
    // Add your button action here
}





// Main loop function (call this regularly)
static void gui_update() {
    update_button(&test_button, "Click Me");
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
    draw_button(&test_button, "Click Me");

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










