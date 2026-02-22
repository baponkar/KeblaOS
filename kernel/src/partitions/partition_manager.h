#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "mbr.h"
#include "guid.h"
#include "gpt.h"


typedef enum {
    ESP_TYPE,
    LINUX_TYPE,
    MS_RESERVED_TYPE, 
    WINDOWS_MSR_TYPE,
    WINDOWS_RECOVERY
} PARTITION_TYPE;


void generate_guid(uint8_t guid[16], uint8_t cpu_id);








