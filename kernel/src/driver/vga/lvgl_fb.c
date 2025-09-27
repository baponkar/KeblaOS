

#include "../../lib/time.h"
#include "../../lib/stdio.h"
#include "../../memory/kheap.h"

#include "../../sys/timer/tsc.h"
#include "../../lib/time.h"

#include "../../driver/vga/framebuffer.h"
#include "../../driver/vga/vga.h"
#include "../../driver/vga/color.h"

#include "../../../../ext_lib/lvgl-9.3.0/lvgl.h"
#include "../../../../ext_lib/lvgl-9.3.0/src/display/lv_display.h"

#include "lvgl_fb.h"

uint32_t MY_DISP_WIDTH;
uint32_t MY_DISP_HEIGHT;



/* Declare buffer for 1/10 screen size; BYTES_PER_PIXEL will be 2 for RGB565. */
#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_XRGB8888))


void my_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    /* The most simple case (also the slowest) to send all rendered pixels to the
     * screen one-by-one.  `put_px` is just an example.  It needs to be implemented by you. */
    uint32_t * buf32 = (uint32_t *)px_map; /* Let's say it's a 16 bit (RGB565) display */
    int32_t x, y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            set_pixel(x, y, *buf32);
            buf32++;
        }
    }

    /* IMPORTANT!!!
     * Inform LVGL that flushing is complete so buffer can be modified again. */
    lv_display_flush_ready(display);
}



void lvgl_test() {

    lv_init();

    MY_DISP_WIDTH = get_fb0_width();
    MY_DISP_HEIGHT = get_fb0_height();

    lv_display_t * display1 = lv_display_create(MY_DISP_WIDTH, MY_DISP_HEIGHT);

    
    lv_display_set_flush_cb(display1, my_flush_cb);

    size_t buf_bytes = MY_DISP_WIDTH * MY_DISP_HEIGHT / 10 * BYTES_PER_PIXEL;
    uint8_t *buf1 = (uint8_t *)kheap_alloc(buf_bytes, ALLOCATE_DATA);
    if(!buf1){
        printf("buf1 allocation failed!\n");
        return;
    }

    // Set display buffer for display `display1`.
    lv_display_set_buffers(display1, (void *) buf1, NULL, buf_bytes, LV_DISPLAY_RENDER_MODE_FULL);

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello LVGL!");
    lv_obj_center(label);


    while (1) {
        // lv_timer_handler();
        usleep(0, 5000);
    }
}






