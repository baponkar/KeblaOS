#include "../../../lvgl-9.2.2/lvgl.h"
#include "vga_term.h"       // Header for VGA functions (e.g., vga_init, set_pixel, swap_buffers)
#include "vga_gfx.h"       // Header for graphics functions (e.g., draw_line, draw_rectangle)
#include "vga_settings.h"
#include "framebuffer.h"
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../x86_64/timer/apic_timer.h" // apic_delay

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