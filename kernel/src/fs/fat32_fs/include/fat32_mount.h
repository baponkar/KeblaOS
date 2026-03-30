#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


bool create_fat32_volume( uint32_t start_lba, uint32_t sectors);
bool fat32_mount(int disk_no, uint64_t start_lba, char *vol_label);

void fat32_reset();