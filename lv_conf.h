#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define LV_INTTYPES_INCLUDE    <stdint.h>


#define LV_COLOR_DEPTH     32 // For 32-bit framebuffer (Limine gives 32bpp framebuffers)
#define LV_USE_OS          LV_OS_NONE

#define LV_CONF_INCLUDE_SIMPLE 1


#define LV_PRId32 "d"
#define LV_PRID64 "ld"
#define LV_PRIdPTR "d"
#define LV_PRId8 "d"
#define LV_PRId16 "d"


