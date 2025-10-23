
/*
Virtual File System

References:
    https://wiki.osdev.org/VFS

*/

#include "../driver/disk/disk.h"                

#include "../fs/FatFs-R0.15b/source/ff.h"
#include "../fs/FatFs-R0.15b/source/diskio.h"

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/limit.h"
#include "../lib/errno.h"

#include "vfs.h"



#define SECTOR_SIZE 512


FS_TYPE detect_filesystem(int disk_no) {
    uint8_t sector[SECTOR_SIZE];

    // --- Step 1: read LBA 0 (MBR or Boot Sector) ---
    if (!kebla_disk_read(disk_no, 0, 1, sector))
        return VFS_UNKNOWN;

    // --- Step 2: check MBR signature (0x55AA) ---
    bool has_mbr = (sector[510] == 0x55 && sector[511] == 0xAA);
    if (has_mbr && sector[0x1BE + 4] != 0x00) {
        // Partition type field
        uint8_t ptype = sector[0x1BE + 4];
        uint32_t start_lba = *(uint32_t*)&sector[0x1BE + 8];

        // Read first sector of partition
        uint8_t pboot[SECTOR_SIZE];
        if (kebla_disk_read(disk_no, start_lba, 1, pboot)) {
            if (memcmp(&pboot[0x52], "FAT32", 5) == 0 ||
                memcmp(&pboot[0x36], "FAT32", 5) == 0)
                return VFS_FAT32;
            if (memcmp(&pboot[0x36], "FAT16", 5) == 0)
                return VFS_FAT16;
            if (memcmp(&pboot[0x36], "FAT12", 5) == 0)
                return VFS_FAT12;
        }
    }

    // --- Step 3: Check raw boot sector (superfloppy case) ---
    if (memcmp(&sector[0x52], "FAT32", 5) == 0 ||
        memcmp(&sector[0x36], "FAT32", 5) == 0)
        return VFS_FAT32;

    if (memcmp(&sector[0x36], "FAT16", 5) == 0)
        return VFS_FAT16;

    if (memcmp(&sector[0x36], "FAT12", 5) == 0)
        return VFS_FAT12;

    // --- exFAT ---
    if (memcmp(&sector[0x03], "EXFAT   ", 8) == 0)
        return VFS_EXFAT;

    // --- NTFS ---
    if (memcmp(&sector[0x03], "NTFS    ", 8) == 0)
        return VFS_NTFS;

    // --- ext2/3/4 (superblock at 1024 bytes) ---
    uint8_t extbuf[1024 + SECTOR_SIZE];
    if (kebla_disk_read(disk_no, 2, 2, extbuf)) {
        uint16_t magic = *(uint16_t*)&extbuf[1024 + 0x38];
        if (magic == 0xEF53) return VFS_EXT2;
    }

    // --- ISO9660 ---
    uint8_t sector16[SECTOR_SIZE];
    if (kebla_disk_read(disk_no, 16, 1, sector16)) {
        if (memcmp(&sector16[0x01], "CD001", 5) == 0)
            return VFS_ISO9660;
    }

    return VFS_RAW;
}










