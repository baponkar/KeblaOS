
#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"

#include "../../../driver/disk/disk.h"
#include "../include/diskio.h"


int disk_no = 1;

bool fat32_disk_read(uint64_t lba, uint32_t count, void* buffer) {
    // bool kebla_disk_read(int disk_no, uint64_t lba, uint32_t count, void* buf);
    return kebla_disk_read(disk_no, lba, count, buffer);
}

bool fat32_disk_write(uint64_t lba, uint32_t count, const void* buffer) {
    return kebla_disk_write(disk_no, lba, count, buffer);
}




