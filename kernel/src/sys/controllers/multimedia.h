#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>




typedef enum {
    MULTIMEDIA_SUBCLASS_VIDEO = 0x0,
    MULTIMEDIA_SUBCLASS_AUDIO = 0x1,
    MULTIMEDIA_SUBCLASS_TELEPHONY = 0x2,
    MULTIMEDIA_SUBCLASS_HD_AUDIO = 0x3,
    MULTIMEDIA_SUBCLASS_OTHER = 0x80
}MULTIMEDIA_SUBCLASS_CODE;