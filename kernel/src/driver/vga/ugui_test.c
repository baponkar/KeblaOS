


#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/math.h"

#include "framebuffer.h"
#include "vga.h"

#include "../mouse/mouse.h"

#include "ugui_test.h"


// VGA mode width/height
uint64_t SCREEN_WIDTH;
uint64_t SCREEN_HEIGHT;

Button test_button;

UG_GUI gui;

void setpixel(UG_S16 x, UG_S16 y, UG_COLOR color){
    set_pixel((int)x, (int)y, (uint32_t) color);
}

void gui_init()
{
    SCREEN_WIDTH = get_fb0_width();
    SCREEN_HEIGHT = get_fb0_height();
    
    UG_Init((UG_GUI*)&gui, setpixel, SCREEN_WIDTH, SCREEN_HEIGHT);
}

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

void ugui_test_1(){
    gui_init();

    UG_FillScreen(C_BLACK); // Clear screen

    // Initialize button
    test_button.x = 100;
    test_button.y = 100;
    test_button.width = 120;
    test_button.height = 40;
    test_button.pressed = false;
    test_button.on_click = button_callback;
    

    UG_DrawFrame(50, 50, 550, 550, C_WHITE);   // rectangle
    UG_FillFrame(60, 60, 540, 540, C_BLUE);    // filled rectangle

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
    }
}

// Main loop function (call this regularly)
void gui_update() {
    update_button(&test_button);
    UG_Update();
}

// =================================================================================================

// External declarations from your existing code
extern uint64_t SCREEN_WIDTH;
extern uint64_t SCREEN_HEIGHT;
extern UG_GUI gui;

// Function prototypes
void window_callback(UG_MESSAGE* msg);

// Window and objects - using different object IDs
UG_WINDOW main_window;
UG_OBJECT objects[5]; // Button, Checkbox, Textbox, Image, Button2

// Object data
UG_BUTTON button1;
UG_CHECKBOX checkbox1;
UG_TEXTBOX textbox1;
UG_IMAGE image1;
UG_BUTTON button2;

// Use different object IDs to avoid conflicts
#define MY_BUTTON_1_ID    OBJ_ID_0
#define MY_CHECKBOX_ID    OBJ_ID_1  // Different from button ID
#define MY_TEXTBOX_ID     OBJ_ID_2
#define MY_IMAGE_ID       OBJ_ID_3
#define MY_BUTTON_2_ID    OBJ_ID_4

// Bitmap data (simple 16x16 example)
const UG_U8 bitmap_data[] = {
    // Simple checkerboard pattern
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
};

UG_BMP test_bmp = {
    (void*)bitmap_data,
    16, 16,  // width, height
    1,       // bpp
    2        // colors
};

// Window callback function
void window_callback(UG_MESSAGE* msg) {
    switch(msg->type) {
        case MSG_TYPE_OBJECT:
            switch(msg->id) {
                case MY_BUTTON_1_ID: // Button 1
                    if(msg->event == OBJ_EVENT_CLICKED) {
                        UG_TextboxSetText(&main_window, MY_TEXTBOX_ID, "Button 1 Clicked!");
                    }
                    break;
                    
                case MY_BUTTON_2_ID: // Button 2
                    if(msg->event == OBJ_EVENT_CLICKED) {
                        UG_TextboxSetText(&main_window, MY_TEXTBOX_ID, "Button 2 Clicked!");
                    }
                    break;
                    
                case MY_CHECKBOX_ID: // Checkbox
                    if(msg->event == OBJ_EVENT_CLICKED) {
                        UG_U8 checked = UG_CheckboxGetChecked(&main_window, MY_CHECKBOX_ID);
                        if(checked) {
                            UG_TextboxSetText(&main_window, MY_TEXTBOX_ID, "Checkbox Checked");
                        } else {
                            UG_TextboxSetText(&main_window, MY_TEXTBOX_ID, "Checkbox Unchecked");
                        }
                    }
                    break;
                    
                default:
                    // Handle other object IDs if needed
                    break;
            }
            break;
            
        default:
            // Handle other message types if needed
            break;
    }
}

// Safe font selection function
void select_available_font(void) {
    // Try to select the best available font
    #ifdef USE_FONT_8X12
    UG_FontSelect(&FONT_8X12);
    #elif defined(USE_FONT_8X8)
    UG_FontSelect(&FONT_8X8);
    #elif defined(USE_FONT_7X12)
    UG_FontSelect(&FONT_7X12);
    #elif defined(USE_FONT_6X10)
    UG_FontSelect(&FONT_6X10);
    #elif defined(USE_FONT_5X12)
    UG_FontSelect(&FONT_5X12);
    #elif defined(USE_FONT_5X8)
    UG_FontSelect(&FONT_5X8);
    #elif defined(USE_FONT_4X6)
    UG_FontSelect(&FONT_4X6);
    #else
    // If no fonts are available, you might need to define one
    // For now, we'll just proceed without setting a font
    // The library might have a default font
    #endif
}

// Initialize the GUI window and elements
void create_gui_window(void) {
    // Calculate window position and size relative to screen dimensions
    UG_S16 window_margin = 40;
    UG_S16 window_width = SCREEN_WIDTH - 2 * window_margin;
    UG_S16 window_height = SCREEN_HEIGHT - 2 * window_margin;
    
    // Set colors
    UG_SetForecolor(C_WHITE);
    UG_SetBackcolor(C_BLUE);
    
    // Select font using safe function
    select_available_font();
    
    // Create main window
    UG_RESULT result = UG_WindowCreate(&main_window, objects, 5, window_callback);
    if(result != UG_RESULT_OK) {
        // Handle error - maybe reduce number of objects or check memory
        return;
    }
    
    // Configure window
    UG_WindowResize(&main_window, window_margin, window_margin, 
                   window_margin + window_width, window_margin + window_height);
    UG_WindowSetTitleText(&main_window, "VGA µGUI Demo");
    UG_WindowSetStyle(&main_window, WND_STYLE_3D | WND_STYLE_SHOW_TITLE);
    UG_WindowSetForeColor(&main_window, C_BLACK);
    UG_WindowSetBackColor(&main_window, C_LIGHT_GRAY);
    
    // Calculate element positions based on window size
    UG_S16 btn_width = 120;
    UG_S16 btn_height = 30;
    UG_S16 elem_spacing = 20;
    UG_S16 start_y = 50;
    
    // Create Button 1
    result = UG_ButtonCreate(&main_window, &button1, MY_BUTTON_1_ID, 
                   window_margin + 40, start_y, 
                   window_margin + 40 + btn_width, start_y + btn_height);
    if(result == UG_RESULT_OK) {
        UG_ButtonSetText(&main_window, MY_BUTTON_1_ID, "Click Me");
        UG_ButtonSetForeColor(&main_window, MY_BUTTON_1_ID, C_WHITE);
        UG_ButtonSetBackColor(&main_window, MY_BUTTON_1_ID, C_BLUE);
        UG_ButtonSetAlternateForeColor(&main_window, MY_BUTTON_1_ID, C_BLACK);
        UG_ButtonSetAlternateBackColor(&main_window, MY_BUTTON_1_ID, C_LIGHT_BLUE);
        UG_ButtonSetStyle(&main_window, MY_BUTTON_1_ID, BTN_STYLE_3D);
    }
    
    // Create Checkbox
    result = UG_CheckboxCreate(&main_window, &checkbox1, MY_CHECKBOX_ID, 
                     window_margin + 180, start_y, 
                     window_margin + 180 + btn_width, start_y + btn_height);
    if(result == UG_RESULT_OK) {
        UG_CheckboxSetText(&main_window, MY_CHECKBOX_ID, "Enable Feature");
        UG_CheckboxSetForeColor(&main_window, MY_CHECKBOX_ID, C_BLACK);
        UG_CheckboxSetBackColor(&main_window, MY_CHECKBOX_ID, C_LIGHT_GRAY);
    }
    
    // Create Textbox (wider to span both columns)
    result = UG_TextboxCreate(&main_window, &textbox1, MY_TEXTBOX_ID, 
                    window_margin + 40, start_y + btn_height + elem_spacing,
                    window_margin + window_width - 40, start_y + btn_height + elem_spacing + 40);
    if(result == UG_RESULT_OK) {
        UG_TextboxSetText(&main_window, MY_TEXTBOX_ID, "Welcome to VGA µGUI Demo!");
        UG_TextboxSetForeColor(&main_window, MY_TEXTBOX_ID, C_BLUE);
        UG_TextboxSetBackColor(&main_window, MY_TEXTBOX_ID, C_WHITE);
        UG_TextboxSetAlignment(&main_window, MY_TEXTBOX_ID, ALIGN_CENTER);
    }
    
    // Create Image
    UG_S16 image_size = 64;
    result = UG_ImageCreate(&main_window, &image1, MY_IMAGE_ID, 
                  window_margin + 40, start_y + btn_height * 2 + elem_spacing * 2,
                  window_margin + 40 + image_size, start_y + btn_height * 2 + elem_spacing * 2 + image_size);
    if(result == UG_RESULT_OK) {
        UG_ImageSetBMP(&main_window, MY_IMAGE_ID, &test_bmp);
    }
    
    // Create Button 2
    result = UG_ButtonCreate(&main_window, &button2, MY_BUTTON_2_ID, 
                   window_margin + 180, start_y + btn_height * 2 + elem_spacing * 2,
                   window_margin + 180 + btn_width, start_y + btn_height * 3 + elem_spacing * 2);
    if(result == UG_RESULT_OK) {
        UG_ButtonSetText(&main_window, MY_BUTTON_2_ID, "Exit");
        UG_ButtonSetForeColor(&main_window, MY_BUTTON_2_ID, C_WHITE);
        UG_ButtonSetBackColor(&main_window, MY_BUTTON_2_ID, C_RED);
        UG_ButtonSetStyle(&main_window, MY_BUTTON_2_ID, BTN_STYLE_3D);
    }
    
    // Show the window
    UG_WindowShow(&main_window);
}

// Update function to call in your main loop
// void gui_update(void) {
    // Handle touch input if available
    // You can implement mouse-to-touch conversion for VGA
    /*
    static UG_S16 last_x = 0, last_y = 0;
    static UG_U8 last_state = 0;
    
    // Get mouse position and convert to touch
    UG_U8 touch_state = (mouse_buttons & 1) ? TOUCH_STATE_PRESSED : TOUCH_STATE_RELEASED;
    if(mouse_x != last_x || mouse_y != last_y || touch_state != last_state) {
        UG_TouchUpdate(mouse_x, mouse_y, touch_state);
        last_x = mouse_x;
        last_y = mouse_y;
        last_state = touch_state;
    }
    */
    
    // Update GUI
//     UG_Update();
// }

// Cleanup function
void gui_cleanup(void) {
    UG_WindowDelete(&main_window);
}


void ugui_test_2(){
    gui_init();          // Your existing initialization
    create_gui_window(); // Create the GUI window
    
    while(1) {
        gui_update();    // Update GUI in main loop
        // Your other code here
    }
    
    gui_cleanup();       // Cleanup before exit
}