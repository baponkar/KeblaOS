#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    VFS_UNKNOWN = 0,
    VFS_FAT12 = 1,
    VFS_FAT16 = 2,
    VFS_FAT32 = 3,
    VFS_EXFAT = 4,
    VFS_EXT2 = 5,
    VFS_EXT3 = 6,
    VFS_EXT4 = 7,
    VFS_NTFS = 8,
    VFS_UDF = 9,
    VFS_NFS = 10,
    VFS_SMB = 11,
    VFS_SSHFS = 12,
    VFS_WEBDEV = 13,
    VFS_APFS = 14,
    VFS_ISO9660 = 15,
    VFS_TEMPFS = 16,
    VFS_RAW = 17
}FS_TYPE;


typedef struct {
    FS_TYPE type;
    const char* name;
    uint64_t start_lba;
    uint32_t sector_size;
    bool valid;
} FS_INFO;


FS_TYPE detect_filesystem(int disk_no);

