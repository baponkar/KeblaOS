#include "fatfs_utils.h"

uint32_t get_fattime(void) {
    // Return a fixed time (2025-01-01 00:00:00) for now
    return ((2025 - 1980) << 25) | (1 << 21) | (1 << 16);
    // Format: [31:25] year - 1980, [24:21] month, [20:16] day
    //         [15:11] hour, [10:5] minute, [4:0] second / 2
}
