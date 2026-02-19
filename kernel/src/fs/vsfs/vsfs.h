#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define DIRECT_PTRS 15

typedef struct __attribute__((packed)){
    uint16_t mode;  // can this file read/write/executed
    uint16_t uid;   // Who owns this file
    uint32_t size;  // how many bytes are in this bfile
    uint32_t atime; // last access time
    uint32_t ctime; // change time
    uint32_t mtime; // modification time
    uint32_t dtime; // Delete time
    uint16_t gid;   // group id
    uint16_t lcnt;  // link count
    uint32_t blocks;// how many blocs allocated for this file
    uint32_t flags; // how should this fs use this inode
    uint32_t osdf;  // os dependent field
    uint32_t block[DIRECT_PTRS];    // a set of disk pointers : total 15 pointers of 4 bytes each
    uint32_t indirect;              // single indirect
    uint32_t double_indirect;       // double indirect
    uint32_t triple_indirect;       // triple indirect
    
    uint32_t version;
}inode_t;           // 104 bytes


typedef struct __attribute__((packed)){
    uint32_t magic;                 // Magic Number: 0x20240218
    uint8_t  name[8];               // A 7 byte string
    uint32_t start_lba;             // Starting LBA of this FS
    uint32_t byte_per_block;        // Block size in bytes
    uint32_t total_blocks;          // Total blocks acquired by this FS
    uint32_t reserved_blocks;       // Total reserved blocks by 'superblock, inode bitmap, data bitmap, inodes'
    uint32_t total_inodes;          // Total inodes present in inode table

    uint32_t num_super_block;       // Total blocks acquired by superblock
    uint32_t num_inode_bitmap;      // Total blocks acquired by inode bitmap
    uint32_t num_inode_table;       // Total Blocks acquired by inode table 
    uint32_t num_data_bitmap;       // Total Blocks acquire by data bitmap table

    uint32_t inode_bitmap_block;    // Start Block no of Inode Bitmap
    uint32_t data_bitmap_block;     // Start Block no of Data Bitmap
    uint32_t inode_table_block;     // Start Block no of Inode table
    uint32_t data_region_block;     // Start Block no of Data block
} superblock_t;                     // 64 Bytes 


bool vsfs_mkfs(int disk_no, uint64_t start_lba, uint32_t total_sectors);



