
#include "../../driver/disk/disk.h"

#include "../../lib/stdio.h"
#include "../../lib/stdlib.h"
#include "../../lib/string.h"

#include "ext2.h"


static bool ext2_read_block(uint32_t block, void* buf) {
    uint64_t lba = (uint64_t)block * (ext2.block_size / 512);
    uint32_t count = ext2.block_size / 512;
    return kebla_disk_read(ext2.disk_no, lba, count, buf);
}

static bool ext2_write_block(uint32_t block, void* buf) {
    uint64_t lba = (uint64_t)block * (ext2.block_size / 512);
    uint32_t count = ext2.block_size / 512;
    return kebla_disk_write(ext2.disk_no, lba, count, buf);
}

bool ext2_mount(int disk_no) {

    ext2.disk_no = disk_no;

    uint8_t buffer[1024];

    // Superblock is always at byte offset 1024
    if (!kebla_disk_read(disk_no, 2, 2, buffer))
        return false;

    ext2.super = *(ext2_superblock_t*)buffer;

    if (ext2.super.s_magic != 0xEF53)
        return false;

    ext2.block_size = 1024 << ext2.super.s_log_block_size;

    uint32_t group_count = (ext2.super.s_blocks_count + ext2.super.s_blocks_per_group - 1) / ext2.super.s_blocks_per_group;

    ext2.groups = malloc(group_count * sizeof(ext2_group_desc_t));

    uint32_t gd_block = (ext2.block_size == 1024) ? 2 : 1;
    ext2_read_block(gd_block, ext2.groups);

    return true;
}

bool ext2_read_inode(uint32_t inode_no, ext2_inode_t* inode) {

    uint32_t group =  (inode_no - 1) / ext2.super.s_inodes_per_group;

    uint32_t index =  (inode_no - 1) % ext2.super.s_inodes_per_group;

    uint32_t inode_table =  ext2.groups[group].bg_inode_table;

    uint32_t inode_size = ext2.super.s_inode_size;

    uint32_t block =  inode_table + (index * inode_size) / ext2.block_size;

    uint32_t offset = (index * inode_size) % ext2.block_size;

    uint8_t buf[ext2.block_size];

    if (!ext2_read_block(block, buf))
        return false;

    *inode = *(ext2_inode_t*)(buf + offset);
    return true;
}

bool ext2_read_file(ext2_inode_t* inode, uint8_t* buffer) {

    uint32_t remaining = inode->i_size;
    uint32_t block_size = ext2.block_size;
    uint32_t buf_offset = 0;

    // 12 direct blocks
    for (int i = 0; i < 12 && remaining > 0; i++) {

        if (inode->i_block[i] == 0)
            break;

        ext2_read_block(inode->i_block[i], buffer + buf_offset);

        buf_offset += block_size;
        remaining -= block_size;
    }

    // Single indirect
    if (remaining > 0 && inode->i_block[12]) {

        uint32_t pointers[block_size / 4];
        ext2_read_block(inode->i_block[12], pointers);

        for (uint32_t i = 0;
             i < block_size / 4 && remaining > 0; i++) {

            if (!pointers[i]) break;

            ext2_read_block(pointers[i],  buffer + buf_offset);

            buf_offset += block_size;
            remaining -= block_size;
        }
    }

    return true;
}

void ext2_list_dir(uint32_t inode_no) {

    ext2_inode_t inode;
    ext2_read_inode(inode_no, &inode);

    uint8_t buf[ext2.block_size];
    ext2_read_block(inode.i_block[0], buf);

    uint32_t offset = 0;

    while (offset < inode.i_size) {

        ext2_dir_entry_t* entry =  (ext2_dir_entry_t*)(buf + offset);

        if (entry->inode != 0) {

            char name[256];
            memcpy(name, entry->name, entry->name_len);
            name[entry->name_len] = 0;

            printf("Name: %s Inode: %d\n", name, entry->inode);
        }

        offset += entry->rec_len;
    }
}


#define SECTOR_SIZE 512
#define EXT2_BLOCK_SIZE 1024
#define SECTORS_PER_BLOCK 2

#define EXT2_SUPER_MAGIC 0xEF53
#define EXT2_ROOT_INO 2
#define EXT2_GOOD_OLD_REV 0
#define EXT2_NDIR_BLOCKS 12

bool create_ext2_fs(int disk_no, uint64_t start_lba, uint64_t total_sectors)
{
    if (total_sectors < 100){
        printf("[EXT2] Not enough space to create EXT2 filesystem!\n");
        return false;
    }

    uint32_t total_blocks =  total_sectors / SECTORS_PER_BLOCK;

    uint32_t total_inodes = 128;   // simple fixed count

    uint8_t block[EXT2_BLOCK_SIZE];
    memset(block, 0, EXT2_BLOCK_SIZE);

    // --------------------------------------------------
    // 1️⃣ SUPERBLOCK (block 1)
    // --------------------------------------------------

    ext2_superblock_t *sb = (ext2_superblock_t*)block;

    sb->s_inodes_count      = total_inodes;
    sb->s_blocks_count      = total_blocks;
    sb->s_r_blocks_count    = 0;
    sb->s_free_blocks_count = total_blocks - 10;
    sb->s_free_inodes_count = total_inodes - 1;
    sb->s_first_data_block  = 1;
    sb->s_log_block_size    = 0; // 1024
    sb->s_blocks_per_group  = total_blocks;
    sb->s_inodes_per_group  = total_inodes;
    sb->s_magic             = EXT2_SUPER_MAGIC;
    sb->s_rev_level         = EXT2_GOOD_OLD_REV;
    sb->s_inode_size        = 128;
    sb->s_first_ino         = 11;

    if(!kebla_disk_write(disk_no, start_lba + 2, 2, block)){
        printf("[EXT2] Failed to write Superblock!\n");
        return false;
    }

    memset(block, 0, EXT2_BLOCK_SIZE);

    // --------------------------------------------------
    // 2️⃣ GROUP DESCRIPTOR (block 2)
    // --------------------------------------------------

    ext2_group_desc_t *gd = (ext2_group_desc_t*)block;

    gd->bg_block_bitmap = 3;
    gd->bg_inode_bitmap = 4;
    gd->bg_inode_table  = 5;
    gd->bg_free_blocks_count = total_blocks - 10;
    gd->bg_free_inodes_count = total_inodes - 1;
    gd->bg_used_dirs_count = 1;

    if(!kebla_disk_write(disk_no, start_lba + 4, 2, block)){
        printf("[EXT2] Failed to write Group Descriptor!\n");
        return false;
    }

    // --------------------------------------------------
    // 3️⃣ BLOCK BITMAP (block 3)
    // --------------------------------------------------

    memset(block, 0, EXT2_BLOCK_SIZE);

    // Mark first 10 blocks used
    for (int i = 0; i < 10; i++){
        block[i / 8] |= (1 << (i % 8));
    }

    if(!kebla_disk_write(disk_no, start_lba + 6, 2, block)){
        printf("[EXT2] Failed to write Block Bitmap!\n");
        return false;
    }

    // --------------------------------------------------
    // 4️⃣ INODE BITMAP (block 4)
    // --------------------------------------------------

    memset(block, 0, EXT2_BLOCK_SIZE);

    // Mark inode 1 (bad) and inode 2 (root) used
    block[0] |= 0x06;

    if(!kebla_disk_write(disk_no, start_lba + 8, 2, block)){
        printf("[EXT2] Failed to write Inode Bitmap!\n");
        return false;
    }

    // --------------------------------------------------
    // 5️⃣ INODE TABLE (block 5+)
    // --------------------------------------------------

    memset(block, 0, EXT2_BLOCK_SIZE);

    ext2_inode_t *inode_table = (ext2_inode_t*)block;

    ext2_inode_t *root = &inode_table[1]; // inode 2

    root->i_mode = 0x4000 | 0755; // directory
    root->i_uid  = 0;
    root->i_gid  = 0;
    root->i_size = EXT2_BLOCK_SIZE;
    root->i_links_count = 2;
    root->i_blocks = 2;
    root->i_block[0] = 9; // first data block

    if(!kebla_disk_write(disk_no, start_lba + 10, 2, block)){
        printf("[EXT2] Failed to write Root Inode!\n");
        return false;
    }

    // --------------------------------------------------
    // 6️⃣ ROOT DIRECTORY DATA BLOCK (block 9)
    // --------------------------------------------------

    memset(block, 0, EXT2_BLOCK_SIZE);

    ext2_dir_entry_t *dot = (ext2_dir_entry_t*)block;

    dot->inode = EXT2_ROOT_INO;
    dot->rec_len = 12;
    dot->name_len = 1;
    dot->file_type = 2;
    dot->name[0] = '.';

    ext2_dir_entry_t *dotdot = (ext2_dir_entry_t*)(block + 12);

    dotdot->inode = EXT2_ROOT_INO;
    dotdot->rec_len = EXT2_BLOCK_SIZE - 12;
    dotdot->name_len = 2;
    dotdot->file_type = 2;
    dotdot->name[0] = '.';
    dotdot->name[1] = '.';

    if(!kebla_disk_write(disk_no, start_lba + (9 * 2), 2, block)){
        printf("[EXT2] Failed to write Root Directory Data Block!\n");
        return false;
    }

    return true;
}