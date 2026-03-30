
#pragma once

// The following header files are present in gcc even in -ffreestanding mode
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef struct __attribute__((packed)) {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    uint32_t s_first_ino;
    uint16_t s_inode_size;
} ext2_superblock_t;        // 90 Bytes


typedef struct __attribute__((packed)) {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint32_t bg_reserved[3];
} ext2_group_desc_t;        // 32 Bytes

#define EXT2_NDIR_BLOCKS 12

typedef struct __attribute__((packed)) {
    uint16_t i_mode;        // file type and permissions
    uint16_t i_uid;         // owner user ID
    uint32_t i_size;        // file size in bytes
    uint32_t i_atime;       // access time
    uint32_t i_ctime;       // creation time
    uint32_t i_mtime;       // modification time
    uint32_t i_dtime;       // deletion time
    uint16_t i_gid;         // owner group ID
    uint16_t i_links_count; // number of hard links
    uint32_t i_blocks;      // number of 512-byte blocks allocated
    uint32_t i_flags;       // file flags
    uint32_t i_osd1;        // OS dependent 1
    uint32_t i_block[15];  // 12 direct, 1 single, 1 double, 1 triple
    uint32_t i_generation;  // file version (for NFS)
    uint32_t i_file_acl;    // file ACL
    uint32_t i_dir_acl;     // directory ACL
    uint32_t i_faddr;       // fragment address
    uint8_t  extra[12];     // OS dependent 2
} ext2_inode_t;         // 128 Bytes

typedef struct {
    int disk_no;
    uint64_t start_lba;
    uint32_t block_size;
    ext2_superblock_t super;
    ext2_group_desc_t *groups;
} ext2_fs_t;


typedef struct __attribute__((packed)) {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char name[];
} ext2_dir_entry_t;

bool ext2_mount(int disk_no);
void ext2_list_dir(uint32_t inode_no);
bool ext2_read_file(ext2_inode_t* inode, uint8_t* buffer);
bool ext2_read_inode(uint32_t inode_no, ext2_inode_t* inode);

bool create_ext2_fs(int disk_no, uint64_t start_lba, uint64_t total_sectors);

bool ext2_create_dir(uint32_t parent_inode_no, const char *name);