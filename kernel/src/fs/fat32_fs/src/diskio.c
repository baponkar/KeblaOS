#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../../driver/disk/disk.h"
#include "../include/diskio.h"

int disk_no = 1;



bool disk_read(uint64_t lba, uint32_t count, void* buffer) {
    
    return kebla_disk_read(disk_no, lba, count, buffer);
}


bool disk_write(uint64_t lba, uint32_t count, const void* buffer) {
    return kebla_disk_write(disk_no, lba, count, buffer);
}


