
#include "framebuffer.h"
#include "vga.h"

#include "ugui_wrapper.h"


// VGA mode width/height
uint64_t SCREEN_WIDTH;
uint64_t SCREEN_HEIGHT;
uint64_t SCREEN_PITCH;
uint64_t SCREEN_BPP;


static void setpixel(UG_S16 x, UG_S16 y, UG_COLOR color){
    set_pixel((int)x, (int)y, (uint32_t) color);
}


UG_GUI gui;

void gui_init()
{
    SCREEN_WIDTH = get_fb0_width();
    SCREEN_HEIGHT = get_fb0_height();
    SCREEN_PITCH = get_fb0_pitch();
    SCREEN_BPP = get_fb0_bpp();
    
    UG_Init((UG_GUI*)&gui, setpixel, SCREEN_WIDTH, SCREEN_HEIGHT);
}



