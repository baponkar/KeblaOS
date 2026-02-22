#include "../lib/time.h"
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"

#include "../driver/disk/disk.h"

#include "guid.h"



/*
    a example GUID 
    Data1: (4 bytes) 0x67 0x45 0x23 0x01 = 0x67452301 (Little endian)
    Data2: (2 bytes) 0xAB 0x89 = 0xAB89 (Little enadian)
    Data3: (2 bytes) 0xEF 0xCD = 0xEFCD (Little enadian)
    Data4: (8 bytes) 0x01 0x23 0x45 0x67 0x89 0xAB 0xCD 0xEF = 0x123456789ABCDEF (Big Endian)
    Therefore the example GUID equivalent to 67452301-AB89-EFCD-0123-456789ABCDEF
*/


const guid_t DISK_GUID_EXAMPLE = {   // Primary GPT Header will use this GUID
    0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
    0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF
};

// C12A7328-F81F-11D2-BA4B-00A0C93EC93B
// ESP type GUID support FAT32 fs
const guid_t ESP_TYPE_GUID = {
    0x28,0x73,0x2a,0xc1,0x1f,0xf8,0xd2,0x11,
    0xba,0x4b,0x00,0xa0,0xc9,0x3e,0xc9,0x3b
};

// 0FC63DAF-8483-4772-8E79-3D69D8477DE4 
// Linux GUID is using ext2, ext3, ext4, btrfs, xfs, jfs, ReiserFS, f2fs, nilfs, any custom linux fs
const guid_t LINUX_FS_GUID = {
    0xaf,0x3d,0xc6,0x0f,0x83,0x84,0x72,0x47,
    0x8e,0x79,0x3d,0x69,0xd8,0x47,0x7d,0xe4
};

// Windows specific : EBD0A0A2-B9E5-4433-87C0-68B6B72699C7 suppport NTFS exFAT FAT32 (non-ESP) Regular Windows partitions
const guid_t MS_RESERVED_GUID = {
    0x16, 0xE3, 0xC9, 0xE3,
    0x5C, 0x0B,
    0xB8, 0x4D,
    0x81, 0x7D,
    0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE
};

// E3C9E316-0B5C-4DB8-817D-F92DF00215AE Internal Windows GPT management Usually 16 MB Usually 16 MB
const guid_t WINDOWS_MSR_GUID = {
    0x16,0xE3,0xC9,0xE3,
    0x5C,0x0B,
    0xB8,0x4D,
    0x81,0x7D,
    0xF9,0x2D,0xF0,0x02,0x15,0xAE
};

// DE94BBA4-06D1-4D40-A16A-BFD50179D6AC DE94BBA4-06D1-4D40-A16A-BFD50179D6AC ntfs fs
const guid_t WINDOWS_RECOVERY_GUID = {
    0xA4,0xBB,0x94,0xDE,
    0xD1,0x06,
    0x40,0x4D,
    0xA1,0x6A,
    0xBF,0xD5,0x01,0x79,0xD6,0xAC
};


// ESP Partition would use this unique GUID
const guid_t ESP_GUID = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 
    0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00
};

// DATA Partition would use this unique GUID
const guid_t DATA_PARTITION_GUID = {
    0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 
    0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11
};

// This function convert raw GUID into string
void guid_to_string(uint8_t guid[16], char *out)
{
    sprintf(out,
        "%x%x%x%x-"
        "%x%x-"
        "%x%x-"
        "%x%x-"
        "%x%x%x%x%x%x",

        // Data1 (reverse)
        guid[3], guid[2], guid[1], guid[0],

        // Data2 (reverse)
        guid[5], guid[4],

        // Data3 (reverse)
        guid[7], guid[6],

        // Data4 (normal)
        guid[8], guid[9],
        guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]
    );
}




static uint64_t guid_entropy_state = 0;

static uint64_t mix_entropy(uint8_t cpu_id)
{
    uint64_t t1 = (uint64_t)get_time();
    uint64_t t2 = get_uptime_seconds(cpu_id);
    uint64_t t3 = (uint64_t)clock();

    uint64_t mix = t1 ^ (t2 << 21) ^ (t3 << 7);

    // Xorshift64*
    mix ^= mix >> 12;
    mix ^= mix << 25;
    mix ^= mix >> 27;

    guid_entropy_state ^= mix;
    guid_entropy_state *= 0x2545F4914F6CDD1DULL;

    return guid_entropy_state;
}

void generate_guid(uint8_t guid[16], uint8_t cpu_id)
{
    for (int i = 0; i < 16; i += 8) {
        uint64_t r = mix_entropy(cpu_id);

        guid[i + 0] = (r >> 0) & 0xFF;
        guid[i + 1] = (r >> 8) & 0xFF;
        guid[i + 2] = (r >> 16) & 0xFF;
        guid[i + 3] = (r >> 24) & 0xFF;
        guid[i + 4] = (r >> 32) & 0xFF;
        guid[i + 5] = (r >> 40) & 0xFF;
        guid[i + 6] = (r >> 48) & 0xFF;
        guid[i + 7] = (r >> 56) & 0xFF;
    }

    // ---- Set UUID Version (4) ----
    guid[6] = (guid[6] & 0x0F) | 0x40;

    // ---- Set UUID Variant (10xx) ----
    guid[8] = (guid[8] & 0x3F) | 0x80;
}









