#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include  "../ahci/ahci.h"

#define FS_MAGIC 0x46534653 // 'FSFS'
#define BLOCK_SIZE 512
#define INODE_COUNT 1024
#define DIRECT_BLOCKS 12
#define INDIRECT_BLOCKS (BLOCK_SIZE / sizeof(uint32_t))

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t inode_count;
    uint32_t inode_table_start;
    uint32_t inode_table_blocks;
    uint32_t bitmap_start;
    uint32_t bitmap_blocks;
    uint32_t data_start;
} Superblock;

typedef struct __attribute__((packed)) {
    uint32_t mode;
    uint32_t size;
    uint32_t blocks[DIRECT_BLOCKS];
    uint32_t indirect_block;
} Inode;

typedef struct __attribute__((packed)) {
    char name[28];
    uint32_t inode_num;
} DirEntry;

// Function prototypes
bool format_fs(HBA_PORT_T *port);
bool fs_init(HBA_PORT_T *port);
bool read_superblock(HBA_PORT_T *port, Superblock *sb);
bool write_superblock(HBA_PORT_T *port, Superblock *sb);
uint32_t allocate_block(HBA_PORT_T *port);
bool free_block(HBA_PORT_T *port, uint32_t block);
bool get_inode(HBA_PORT_T *port, uint32_t inode_num, Inode *inode);
bool put_inode(HBA_PORT_T *port, uint32_t inode_num, Inode *inode);

int read_file(HBA_PORT_T *port, uint32_t inode_num, uint8_t *buf, uint32_t size, uint32_t offset);
int write_file(HBA_PORT_T *port, uint32_t inode_num, uint8_t *buf, uint32_t size, uint32_t offset);
bool create_file(HBA_PORT_T *port, uint32_t parent_inode, const char *name, uint32_t *new_inode);

bool list_directory(HBA_PORT_T *port, uint32_t inode_num);


void ahci_test(HBA_MEM_T *abar);


