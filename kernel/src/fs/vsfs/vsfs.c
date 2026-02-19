
/*
    Very Simple File System(VSFS)
    Description: A very simple filesystem like ext2 type implemented by me.    
                Total Blocks required for inode bitmaps
                Total inodes = MAX_FILES
                a single bit in bitmap can represent a inode.Total bits required is total inodes.
                Total Bytes = total_inodes / 8 Bytes
                inode bitmap size = (total_inodes / 8) bytes
                Total blocks = ((total_inodes / 8) + BLOCK_SIZE) / BLOCK_SIZE 

    Build Date: 18-02-2026
    Build Time: 11:01 AM
    Developer: Bapon Kar
    Reference: https://pages.cs.wisc.edu/~remzi/OSTEP/file-implementation.pdf
*/

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/time.h"

#include "../../driver/disk/disk.h"

#include "vsfs.h"



#define MAX_FILES 256   // Total number of inodes

#define SECTOR_SIZE 512
#define BLOCK_SIZE 4096 // 4KB
#define TOTAL_BLOCKS(tot_sectors)((tot_sectors) * SECTOR_SIZE / BLOCK_SIZE)

#define FS_MAGIC 0x20240218

#define INODE_BITMAP_BLOCK 1
#define DATA_BITMAP_BLOCK  2
#define INODE_TABLE_BLOCK  3
#define DATA_REGION_BLOCK  7

uint32_t lba_offset;
superblock_t *sb;



// Converting BLOCK to LBA 
static uint32_t block_to_lba(uint32_t block_no){
    uint32_t lba = block_no * BLOCK_SIZE / SECTOR_SIZE;

    return lba;
}

// Converting LBA to Block
static uint32_t lba_to_block(uint32_t lba){
    uint32_t block_no = lba * SECTOR_SIZE / BLOCK_SIZE;

    return block_no;
}

// Reading Blocks
static bool read_block(int disk_no, uint32_t block_no, uint32_t count, void *buff){
    char *buf = (char *) buff;

    uint32_t lba = block_to_lba(block_no);
    uint32_t lba_count = count * BLOCK_SIZE / SECTOR_SIZE;

    return kebla_disk_read(disk_no, lba_offset + lba, lba_count, buf);
}

// Writing Blocks
static bool write_block(int disk_no, uint32_t block_no, uint32_t count, void *buff){
    char *buf = (char *) buff;

    uint32_t lba = block_to_lba(block_no);
    uint32_t lba_count = count * BLOCK_SIZE / SECTOR_SIZE;

    return kebla_disk_write(disk_no, lba_offset + lba, lba_count, buf);
}

// bitmap no = 19, no = 19/8 = 3, offset 3 bitmap no starts from 0
static inline int bitmap_test(uint8_t *bm, int i) {
    return bm[i / 8] & (1 << (i % 8));
}


static inline void bitmap_set(uint8_t *bm, int i) {
    bm[i / 8] |= (1 << (i % 8));
}


static inline void bitmap_clear(uint8_t *bm, int i) {
    bm[i / 8] &= ~(1 << (i % 8));
}


static int vsfs_alloc_inode(uint8_t *inode_bitmap) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!bitmap_test(inode_bitmap, i)) {
            bitmap_set(inode_bitmap, i);
            return i;
        }
    }
    return -1;
}


static int vsfs_alloc_block(uint8_t *data_bitmap, int max_blocks) {
    for (int i = 0; i < max_blocks; i++) {
        if (!bitmap_test(data_bitmap, i)) {
            bitmap_set(data_bitmap, i);
            return i;
        }
    }
    return -1;
}


bool vsfs_mkfs(int disk_no, uint64_t start_lba, uint32_t total_sectors) {

    lba_offset = start_lba;

    uint32_t total_blocks = (total_sectors * SECTOR_SIZE) / BLOCK_SIZE;
    
    uint32_t superblock_blocks = 1;
    uint32_t inode_bitmap_bytes = (MAX_FILES + 7) / 8;
    uint32_t inode_bitmap_blocks = (uint32_t) (( inode_bitmap_bytes + BLOCK_SIZE) / BLOCK_SIZE);
    uint32_t data_bitmap_bytes = (uint32_t)  ((total_blocks - superblock_blocks - inode_bitmap_blocks + 7) / 8);
    uint32_t data_bitmap_blocks = (uint32_t) ((data_bitmap_bytes + BLOCK_SIZE) / BLOCK_SIZE);
    uint32_t inode_table_blocks = (uint32_t) ((MAX_FILES * sizeof(inode_t) + BLOCK_SIZE) / BLOCK_SIZE);
    
    uint32_t reserved_blocks = superblock_blocks + inode_bitmap_blocks + data_bitmap_blocks + inode_table_blocks;
    
    if(total_sectors <= reserved_blocks * BLOCK_SIZE / SECTOR_SIZE){
        printf("Total Sectors is too low to hold VSFS\n");
    }

    sb = (superblock_t *)malloc(sizeof(superblock_t));
    if(!sb) return false;

    sb->magic = FS_MAGIC;
    memcpy(sb->name, "VSFS   ", 8);
    sb->start_lba = start_lba;
    sb->byte_per_block = BLOCK_SIZE;
    sb->total_blocks = total_blocks;
    sb->reserved_blocks = reserved_blocks;
    sb->total_inodes = MAX_FILES;
    sb->num_super_block = 1;
    sb->num_inode_bitmap = inode_bitmap_blocks;
    sb->num_inode_table = inode_table_blocks;
    sb->num_data_bitmap = data_bitmap_blocks;

    sb->inode_bitmap_block = 1;
    sb->data_bitmap_block = sb->inode_bitmap_block + inode_bitmap_blocks;
    sb->inode_table_block = sb->data_bitmap_block + data_bitmap_blocks;
    sb->data_region_block = sb->inode_table_block + inode_table_blocks;

    // Writing superblock
    if(!write_block(disk_no, 0, 1, sb)){
        printf("writing sb failed!\n");
        return false;
    }
    
    // clear inode bitmap block
    uint8_t zero[BLOCK_SIZE * inode_bitmap_blocks];
    memset(zero, 0, BLOCK_SIZE * inode_bitmap_blocks);
    if(!write_block(disk_no,  sb->inode_bitmap_block, sb->num_inode_bitmap, zero)){
        printf("writing inode bitmap failed\n");
        return false;
    }

    // clear data bitmap block
    uint8_t zero_1[BLOCK_SIZE * sb->num_data_bitmap];
    memset(zero_1, 0, BLOCK_SIZE * sb->num_data_bitmap);
    if(!write_block(disk_no, sb->data_bitmap_block, sb->num_data_bitmap, zero_1)){
        printf("writing data bitmap failed!\n");
        return false;
    }

    // clear inode table block
    uint8_t zero_2[BLOCK_SIZE * sb->num_inode_table];
    memset(zero_2, 0, BLOCK_SIZE * sb->num_inode_table);
    if(!write_block(disk_no, sb->inode_table_block, sb->num_inode_table, zero_2)){
        printf("writing inode table failed!\n");
        return false;
    }

    return true;
}


// void vsfs_read_inode(int disk_no, int inum, inode_t *out) {
//     uint32_t offset = inum * sizeof(inode_t);
//     uint32_t block = offset / BLOCK_SIZE;
//     uint32_t off   = offset % BLOCK_SIZE;

//     uint8_t buf[BLOCK_SIZE];

//     if(read_block(disk_no, block, 1, buf)){
//         return;
//     }

//     memcpy(out, buf + off, sizeof(inode_t));
// }


// void vsfs_write_inode(int disk_no, int inum, inode_t *in) {

//     uint32_t offset = inum * sizeof(inode_t);
//     uint32_t block = offset / BLOCK_SIZE;
//     uint32_t off   = offset % BLOCK_SIZE;

//     uint8_t buf[BLOCK_SIZE];

//     if(read_block(disk_no, block, 1, buf)){
//         return;
//     }

//     memcpy(buf + off, in, sizeof(inode_t));

//     if(write_block(disk_no, INODE_TABLE_BLOCK + block, 1, buf)){
//         return;
//     }
// }



// int vsfs_create_inode(int disk_no, uint8_t *inode_bitmap) {

//     int inum = vsfs_alloc_inode(inode_bitmap);
//     if (inum < 0) return -1;

//     inode_t inode;
//     memset(&inode, 0, sizeof(inode));

//     inode.mode = 0x8000;   // regular file
//     inode.size = 0;

//     vsfs_write_inode(disk_no, inum, &inode);
    
//     return inum;
// }


// int vsfs_write_data(int disk_no, inode_t *inode, uint8_t *data_bitmap, int max_blocks,  void *data) {

//     int blk = vsfs_alloc_block(data_bitmap, max_blocks);
//     if (blk < 0) return -1;

//     if(write_block(disk_no, DATA_REGION_BLOCK + blk, max_blocks, data)){
//         return;
//     }

//     inode->dpntrs[0] = blk;
//     inode->size = BLOCK_SIZE;
//     inode->blocks = 1;

//     return 0;
// }




// void vsfs_test(int disk_no) {
//     uint8_t inode_bitmap[BLOCK_SIZE];
//     uint8_t data_bitmap[BLOCK_SIZE];

//     if(write_block(disk_no, DATA_BITMAP_BLOCK, 1, inode_bitmap)){
//         return;
//     }
//     if(write_block(disk_no, DATA_BITMAP_BLOCK, 1, data_bitmap)){
//         return;
//     }

//     int inum = vsfs_create_inode(disk_no, inode_bitmap);

//     inode_t inode;
//     vsfs_read_inode(disk_no, inum, &inode);

//     char buf[BLOCK_SIZE];
//     strcpy(buf, "Hello VSFS");

//     vsfs_write_data(disk_no, &inode, data_bitmap, 64, buf);

//     vsfs_write_inode(disk_no, inum, &inode);

// }







