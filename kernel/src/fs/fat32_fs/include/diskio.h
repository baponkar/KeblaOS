#pragma once
#include <stdint.h>
#include <stdbool.h>

#define SECTOR_SIZE 512



bool disk_read(uint64_t lba, uint32_t count, void* buffer);
bool disk_write(uint64_t lba, uint32_t count, const void* buffer);

void set_disk_no(int no);
int get_current_disk_no();






