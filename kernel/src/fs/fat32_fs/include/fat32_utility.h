#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

bool fat32_read_sector(uint64_t lba, void *buf);
bool fat32_write_sector(uint64_t lba, const void *buf);

bool fat32_read_sectors(uint64_t lba, uint32_t count, void *buf);
bool fat32_write_sectors(uint64_t lba, uint32_t count, const void *buf);


