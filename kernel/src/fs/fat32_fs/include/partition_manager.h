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

typedef struct __attribute__((packed)){
    uint8_t pdrv_no;
    uint8_t partition_no;
    uint64_t start_lba;
    uint64_t sectors;
    guid_t partition_guid;
    guid_t partition_type_guid;
    GPTPartitionEntry gpt_entry; // Store the GPT partition entry for this partition
} PartitionEntry;


bool create_partition(uint8_t pdrv_no, uint64_t start_lba, uint64_t sectors, guid_t partition_guid, guid_t partition_type_guid, char* name);


bool update_partition(size_t partition_index, uint64_t new_start_lba, uint64_t new_sectors, const char* new_name,  uint64_t new_attributes);





