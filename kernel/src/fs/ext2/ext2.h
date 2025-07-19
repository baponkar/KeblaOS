
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>




typedef struct {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t block_size;
    uint32_t inodes_per_group;
    uint32_t blocks_per_group;
    uint32_t first_inode;
    uint32_t inode_size;
} ext2_info_t;

typedef struct {
    uint16_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;      // Operating System Specific value #1
    uint32_t block[15]; // 0-11 direct, 12 single indirect
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint8_t osd2[12]; // Operating System Specific Value #2
} __attribute__((packed)) ext2_inode_t;

typedef struct {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[];
} __attribute__((packed)) ext2_dir_entry_t;

void read_superblock();

bool ext2_init();
void ext2_read_superblock();
void ext2_list_root();

ext2_inode_t ext2_read_inode(uint32_t inode_id);
void ext2_list_dir(uint32_t inode_id);
void ext2_read_file(uint32_t inode_id);
void ext2_test();

void ext2_create_dir(uint32_t parent_inode_id, const char *name);
void ext2_create_file(uint32_t parent_inode_id, const char *name);

uint32_t ext2_path_to_inode(const char *path);

