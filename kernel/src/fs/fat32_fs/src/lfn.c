

#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"


#include "../include/fat32_types.h"
#include "../include/fat32_utility.h"
#include "../include/fat32.h"

#include "../include/lfn.h"



uint8_t fat32_lfn_checksum(uint8_t short_name[11])
{
    uint8_t sum = 0;
    for (int i = 0; i < 11; i++)
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short_name[i];
    return sum;
}

bool fat32_needs_lfn(const char *name)
{
    if (strlen(name) > 12)
        return true;

    for (size_t i = 0; i < strlen(name); i++) {
        if (islower(name[i]) || name[i] == ' ')
            return true;
    }

    return false;
}

