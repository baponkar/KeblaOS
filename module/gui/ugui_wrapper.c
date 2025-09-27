

#include "../libc/include/stdio.h"
#include "../libc/include/string.h"
#include "../libc/include/syscall.h"

#include "ugui_wrapper.h"



static void setpixel(UG_S16 x, UG_S16 y, UG_COLOR color){
    syscall_set_pixel((int)x, (int)y, (uint32_t) color);
}


UG_GUI gui;

void gui_init()
{
    SCREEN_WIDTH = 1200;
    SCREEN_HEIGHT = 800;
    SCREEN_PITCH = 0;
    SCREEN_BPP = 0;
    
    UG_Init((UG_GUI*)&gui, setpixel, SCREEN_WIDTH, SCREEN_HEIGHT);
}



