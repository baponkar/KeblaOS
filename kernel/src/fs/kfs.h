#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>



/* ----------------- CONFIG ----------------- */
#define SECTOR_SIZE         512U
#define FS_BLOCK_SIZE       4096U                 // bytes per FS block
#define SECTORS_PER_BLOCK   (FS_BLOCK_SIZE / SECTOR_SIZE)

#define FS_MAGIC            0x4B425346UL         // "KBSF" kebla simple fs
#define MAX_FILES           128                  // root directory entries
#define FILENAME_MAX_LEN    32

// Layout on-disk (block index relative to FS start block = 0)
#define SB_BLOCK_INDEX      0                    // superblock at block 0
// FAT starts at block 1 and occupies FAT_BLOCKS blocks (computed at format)
#define ROOT_DIR_BLOCKS     4                    // blocks reserved for root dir (tunable)
#define MAX_FAT_ENTRY       0xFFFFFFFFUL         // free marker / end marker used below
#define FAT_FREE            0x00000000UL
#define FAT_EOC             0xFFFFFFFFUL

/* ----------------- ON-DISK STRUCTS ----------------- */
#pragma pack(push,1)
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint64_t total_blocks;    // total blocks belonging to FS area
    uint32_t fat_start;       // block index where FAT starts (relative)
    uint32_t fat_blocks;      // how many blocks FAT occupies
    uint32_t root_start;      // block index where root dir starts
    uint32_t root_blocks;     // number of blocks reserved for root dir
    uint32_t data_start;      // first data block (where file data blocks begin)
    uint8_t  reserved[SECTOR_SIZE - 56]; // pad to sector size (keeps SB < sector)
} fs_superblock_t;

typedef struct {
    char name[FILENAME_MAX_LEN];
    uint32_t first_block;     // index relative to data_start (0 means empty)
    uint64_t size;            // bytes
    uint8_t  flags;           // reserved for future (e.g. in-use)
    uint8_t  reserved[7];     // alignment pad
} fs_dir_entry_t;
#pragma pack(pop)


bool fs_format(int disk_no, uint32_t fs_start_block, uint64_t total_blocks);
bool fs_mount(int disk_no, uint32_t fs_start_block);
bool fs_create(const char *name);
bool fs_write_file(const char *name, const void *data, uint64_t size);
int64_t fs_read_file(const char *name, void *out_buf, uint64_t buf_size);
bool fs_remove(const char *name);
int64_t fs_get_file_size(const char *name);

/* list files (calls a user-provided callback: (name, size)) */
typedef void (*fs_list_cb)(const char *name, uint64_t size, void *user);
void fs_list_files(fs_list_cb cb, void *user);

void fs_unmount();


void kfs_test();
