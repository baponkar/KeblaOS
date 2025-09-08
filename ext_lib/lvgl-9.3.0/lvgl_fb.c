
#include "../../kernel/src/lib/time.h"
#include "../../kernel/src/memory/kheap.h"

#include "lvgl.h"
#include "src/display/lv_display.h"

#include "lvgl_fb.h"

extern void set_pixel(int x, int y, uint32_t color);

static void vga_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    int32_t x, y;

    lv_color_t *color_p = (lv_color_t *)px_map;

    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            lv_color32_t color =  lv_color_to_32(*color_p, LV_OPA_TRANSP);
            uint32_t col = (color_p->red << 16) | (color_p->green << 8) | (color_p->blue);
            set_pixel(x, y, col);
            color_p++;
        }
    }

    lv_disp_flush_ready(disp);  // tell LVGL flush is complete
}


void lvgl_init_vga(int screen_width, int screen_height) {
    lv_init();

    // Create screen buffer
    // static lv_color_t buf1[800*480];
    // Instead of static allocation, consider dynamic allocation if memory is constrained
    lv_color_t *buf1 = (lv_color_t*) kheap_alloc(800 * 480 * sizeof(lv_color_t), ALLOCATE_DATA);
    if (!buf1) {
        // Handle allocation failure
        return;
    }
    
    // Initialize display
    lv_display_t * disp = lv_display_create(screen_width, screen_height);
    lv_display_set_flush_cb(disp, vga_flush);
    lv_display_set_buffers(disp, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
}


void lvgl_test(){
    void lvgl_init_vga(int screen_width, int screen_height);
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello from LVGL + VGA!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    while (1) {
        lv_timer_handler();
        lv_delay_ms(100);
    }
}


