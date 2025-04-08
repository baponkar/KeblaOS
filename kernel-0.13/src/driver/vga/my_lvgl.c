#include "../../../../lvgl-9.2.2/lvgl.h"
#include "vga_term.h"       // Header for VGA functions (e.g., vga_init, set_pixel, swap_buffers)
#include "vga_gfx.h"       // Header for graphics functions (e.g., draw_line, draw_rectangle)
#include "framebuffer.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../x86_64/timer/apic_timer.h" // apic_delay
#include "color.h" // color definitions

#include "../../../../lvgl-9.2.2/src/font/lv_font.h"


#include "my_lvgl.h"

// External framebuffer variables from KeblaOS
extern uint32_t *fb_address;
extern uint64_t fb_width;
extern uint64_t fb_height;
extern uint64_t fb_pitch;
extern uint16_t fb_bpp; // bits per pixel



static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
    // For direct mode, LVGL already draws to your framebuffer
    lv_display_flush_ready(disp);
}

void lvgl_init() {
    lv_init();

    // Create display object
    lv_display_t * disp = lv_display_create(fb_width, fb_height);

    
    // Set resolution
    lv_display_set_resolution(disp, fb_width, fb_height);
    
    // Set color format (ARGB8888 matches your set_pixel format)
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_ARGB8888);
    
    // Set framebuffer with pitch (stride) in BYTES
    lv_display_set_buffers(
        disp, 
        fb_address,    // primary buffer
        NULL,          // no secondary buffer
        fb_pitch * fb_height,  // total buffer size in bytes
        LV_DISPLAY_RENDER_MODE_DIRECT
    );

    // Set flush callback
    lv_display_set_flush_cb(disp, flush_cb);
}

void create_gui() {
    // Create a label on the active screen
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello LVGL!");
    lv_obj_center(label);
}

void create_gui_1() {
    // Create the main screen
    lv_obj_t * scr = lv_scr_act();

    // Set screen background color to blue
    lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BLACK), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // Create a label in the center
    lv_obj_t * label = lv_label_create(scr);
    lv_label_set_text(label, "Welcome to KeblaOS!");
    lv_obj_set_style_text_color(label, lv_color_hex(COLOR_WHITE), 0); // White text
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0); // Big font
    lv_obj_center(label);

    // Create bottom bar container
    lv_obj_t * bottom_bar = lv_obj_create(scr);
    lv_obj_set_size(bottom_bar, lv_pct(100), 60); // Full width, taller height
    lv_obj_align(bottom_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bottom_bar, lv_color_hex(COLOR_LIGHT_GRAY), 0); // White
    lv_obj_set_style_bg_opa(bottom_bar, LV_OPA_80, 0); // 80% transparent
    lv_obj_set_style_radius(bottom_bar, 0, 0); // No rounding
    lv_obj_set_scrollbar_mode(bottom_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(bottom_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create Home button
    lv_obj_t * btn1 = lv_button_create(bottom_bar);
    lv_obj_set_size(btn1, 120, 45);
    lv_obj_set_style_radius(btn1, 12, 0); // Rounded corners
    lv_obj_set_style_shadow_width(btn1, 8, 0); // Soft shadow
    lv_obj_set_style_shadow_color(btn1, lv_color_hex(0x888888), 0);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(COLOR_WHITE), 0); // Blue button
    lv_obj_set_style_text_color(btn1, lv_color_hex(COLOR_BLACK), 0); // White text
    lv_obj_center(btn1);

    lv_obj_t * label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "Home");
    lv_obj_center(label1);

    // Create Settings button
    lv_obj_t * btn2 = lv_button_create(bottom_bar);
    lv_obj_set_size(btn2, 120, 45);
    lv_obj_set_style_radius(btn2, 12, 0); // Rounded corners
    lv_obj_set_style_shadow_width(btn2, 8, 0); // Soft shadow
    lv_obj_set_style_shadow_color(btn2, lv_color_hex(0x888888), 0);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(COLOR_BROWN), 0); // Green button
    lv_obj_set_style_text_color(btn2, lv_color_hex(COLOR_WHITE), 0); // White text
    lv_obj_center(btn2);

    lv_obj_t * label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Settings");
    lv_obj_center(label2);
}


static lv_obj_t * label; // Make it global for animation callback

void anim_cb(void * var, int32_t v) {
    lv_obj_set_x((lv_obj_t *)var, v);
}

void create_gui_animated() {
    lv_obj_t * scr = lv_scr_act();

    // Create label
    label = lv_label_create(scr);
    lv_label_set_text(label, "Welcome to KeblaOS!");

    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0); 
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Set starting position (offscreen left)
    lv_obj_set_x(label, -lv_obj_get_width(label));

    // Create animation
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, label);
    lv_anim_set_values(&a, -lv_obj_get_width(label), 0); // From left offscreen to center
    lv_anim_set_time(&a, 1000); // 1 second duration
    lv_anim_set_exec_cb(&a, anim_cb);

    lv_anim_start(&a);
}

void create_multiple_windows() {
    lv_obj_t * scr = lv_scr_act();

    // ===== Window 1 =====
    lv_obj_t * win1 = lv_win_create(scr);
    lv_obj_set_size(win1, 200, 150);
    lv_obj_align(win1, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_win_add_title(win1, "Window 1");

    lv_obj_t * cont1 = lv_win_get_content(win1);
    lv_obj_t * label1 = lv_label_create(cont1);
    lv_label_set_text(label1, "This is Window 1");
    lv_obj_center(label1);

    // ===== Window 2 =====
    lv_obj_t * win2 = lv_win_create(scr);
    lv_obj_set_size(win2, 200, 150);
    lv_obj_align(win2, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_win_add_title(win2, "Window 2");

    lv_obj_t * cont2 = lv_win_get_content(win2);
    lv_obj_t * label2 = lv_label_create(cont2);
    lv_label_set_text(label2, "This is Window 2");
    lv_obj_center(label2);

    // ===== Window 3 =====
    lv_obj_t * win3 = lv_win_create(scr);
    lv_obj_set_size(win3, 420, 100);
    lv_obj_align(win3, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_win_add_title(win3, "Window 3");

    lv_obj_t * cont3 = lv_win_get_content(win3);
    lv_obj_t * label3 = lv_label_create(cont3);
    lv_label_set_text(label3, "This is Window 3 at Bottom");
    lv_obj_center(label3);
}
