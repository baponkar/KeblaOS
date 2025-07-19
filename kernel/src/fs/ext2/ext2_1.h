#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct ext2_superblock {
    uint32_t s_inodes_count;         // 0x00 Total inode count
    uint32_t s_blocks_count;         // 0x04 Total block count
    uint32_t s_r_blocks_count;       // 0x08 Reserved blocks count
    uint32_t s_free_blocks_count;    // 0x0C Free blocks count
    uint32_t s_free_inodes_count;    // 0x10 Free inodes count
    uint32_t s_first_data_block;     // 0x14 First Data Block
    uint32_t s_log_block_size;       // 0x18 Block size = 1024 << s_log_block_size
    uint32_t s_log_frag_size;        // 0x1C Fragment size = 1024 << s_log_frag_size
    uint32_t s_blocks_per_group;     // 0x20
    uint32_t s_frags_per_group;      // 0x24
    uint32_t s_inodes_per_group;     // 0x28
    uint32_t s_mtime;                // 0x2C Mount time
    uint32_t s_wtime;                // 0x30 Write time
    uint16_t s_mnt_count;            // 0x34 Mount count
    uint16_t s_max_mnt_count;        // 0x36 Max mount count
    uint16_t s_magic;                // 0x38 Magic signature (0xEF53)
    uint16_t s_state;                // 0x3A File system state
    uint16_t s_errors;               // 0x3C Behaviour when detecting errors
    uint16_t s_minor_rev_level;      // 0x3E Minor revision level
    uint32_t s_lastcheck;            // 0x40 Time of last check
    uint32_t s_checkinterval;        // 0x44 Max time between checks
    uint32_t s_creator_os;           // 0x48 OS
    uint32_t s_rev_level;            // 0x4C Revision level
    uint16_t s_def_resuid;           // 0x50 Default UID for reserved blocks
    uint16_t s_def_resgid;           // 0x52 Default GID for reserved blocks

    // Extended fields (Revision 1+)
    uint32_t s_first_ino;            // 0x54 First non-reserved inode
    uint16_t s_inode_size;           // 0x58 Size of inode structure
    uint16_t s_block_group_nr;       // 0x5A Block group # of this superblock
    uint32_t s_feature_compat;       // 0x5C Compatible feature set
    uint32_t s_feature_incompat;     // 0x60 Incompatible feature set
    uint32_t s_feature_ro_compat;    // 0x64 Readonly-compatible feature set
    uint8_t  s_uuid[16];             // 0x68 128-bit UUID
    char     s_volume_name[16];      // 0x78 Volume name
    char     s_last_mounted[64];     // 0x88 Directory where last mounted
    uint32_t s_algorithm_usage_bitmap; // 0xC8 For compression

    // ... Many more optional fields can follow (up to 1024 bytes in total)
} __attribute__((packed)) ext2_superblock_t;


typedef struct ext2_block_group_descriptor {
    uint32_t bg_block_bitmap;       // 0x00 Block ID of block usage bitmap
    uint32_t bg_inode_bitmap;       // 0x04 Block ID of inode usage bitmap
    uint32_t bg_inode_table;        // 0x08 Starting block of inode table
    uint16_t bg_free_blocks_count;  // 0x0C Number of free blocks in group
    uint16_t bg_free_inodes_count;  // 0x0E Number of free inodes in group
    uint16_t bg_used_dirs_count;    // 0x10 Number of directories in group
    uint16_t bg_pad;                // 0x12 Padding
    uint32_t bg_reserved[3];        // 0x14 Reserved
} __attribute__((packed)) ext2_block_group_descriptor_t;


ext2_inode_t read_inode(uint32_t inode_id);
bool init();
void ext2_test_1();