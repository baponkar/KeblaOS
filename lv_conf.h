#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define LV_INTTYPES_INCLUDE    <stdint.h>


#define LV_COLOR_DEPTH     32 // For 32-bit framebuffer (Limine gives 32bpp framebuffers)
#define LV_USE_OS          LV_OS_NONE

#define LV_CONF_INCLUDE_SIMPLE 1

#define LV_FONT_MONTSERRAT_20    1
#define LV_FONT_MONTSERRAT_24    1
#define LV_FONT_MONTSERRAT_26    1
#define LV_FONT_MONTSERRAT_28    1
#define LV_FONT_MONTSERRAT_32    1
#define LV_FONT_MONTSERRAT_34    1
#define LV_FONT_MONTSERRAT_36    1
#define LV_FONT_MONTSERRAT_38    1
#define LV_FONT_MONTSERRAT_40    1


#define LV_PRId32 "d"
#define LV_PRID64 "ld"
#define LV_PRIdPTR "d"
#define LV_PRId8 "d"
#define LV_PRId16 "d"


