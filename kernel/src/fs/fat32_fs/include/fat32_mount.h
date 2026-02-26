#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


bool create_fat32_volume( uint64_t start_lba, uint32_t sectors);
bool fat32_mount( uint64_t start_lba);