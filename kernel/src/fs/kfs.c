
/*
https://0x10.sh/uploads/2021/08/The%20Ext2%20File%20System.pdf
*/

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/time.h"


#include "kfs.h"


// kebla_simplefs.c
// Very small block filesystem for KeblaOS
// Assumptions:
//  - kebla_disk_read/write operate on 512-byte LBAs (sectors).
//  - sector size = 512 bytes.
//  - block size = 4096 bytes (8 sectors per block).
//  - filesystem uses a superblock + FAT table + root-directory in fixed locations.
//  - single-level directory (no subdirs). Fixed max files.
//  - simple contiguous file chain tracked by FAT (like a tiny FAT32).
//
// Replace/adjust constants below if your sector size differs.


extern bool kebla_disk_read(int disk_no, uint64_t lba, uint32_t count, void* buf);
extern bool kebla_disk_write(int disk_no, uint64_t lba, uint32_t count, void* buf);


/* ----------------- IN-MEMORY STATE ----------------- */
static int fs_disk = -1;
static uint64_t fs_total_blocks = 0;
static uint32_t fs_fat_start = 0;
static uint32_t fs_fat_blocks = 0;
static uint32_t fs_root_start = 0;
static uint32_t fs_root_blocks = 0;
static uint32_t fs_data_start = 0;

static uint32_t *fs_fat = NULL;                // loaded FAT (one entry per data block)
static fs_dir_entry_t *fs_root = NULL;         // loaded root directory (in-memory)
static uint32_t fs_num_data_blocks = 0;
static uint32_t fs_max_root_entries = 0;

/* ----------------- helpers ----------------- */

static bool read_blocks(int disk, uint64_t block_index, uint32_t blocks, void *buf) {
    // convert block_index -> LBA sector
    uint64_t lba = (uint64_t)block_index * SECTORS_PER_BLOCK;
    uint32_t count = blocks * SECTORS_PER_BLOCK;
    return kebla_disk_read(disk, lba, count, buf);
}

static bool write_blocks(int disk, uint64_t block_index, uint32_t blocks, const void *buf) {
    uint64_t lba = (uint64_t)block_index * SECTORS_PER_BLOCK;
    uint32_t count = blocks * SECTORS_PER_BLOCK;
    // kebla_disk_write signature takes void* non-const, cast away const
    return kebla_disk_write(disk, lba, count, (void*)buf);
}

static void zero_blocks(int disk, uint64_t block_index, uint32_t blocks) {
    void *tmp = malloc((size_t)blocks * FS_BLOCK_SIZE);
    if (!tmp) return;
    memset(tmp, 0, (size_t)blocks * FS_BLOCK_SIZE);
    write_blocks(disk, block_index, blocks, tmp);
    free(tmp);
}

/* ----------------- formatting & mount ----------------- */

/*
  Format a disk region as this FS.
  disk_no: kebla disk number
  fs_start_block: block index on disk where FS will be created (commonly 0)
  total_blocks: total block count available for FS starting at fs_start_block
*/
bool fs_format(int disk_no, uint32_t fs_start_block, uint64_t total_blocks) {
    // compute FAT size:
    // We must reserve blocks for superblock, FAT, root dir and data.
    // FAT entry is 4 bytes per data block. Let fat_blocks be ceil((num_data_blocks * 4) / block_size).
    // But num_data_blocks depends on fat_blocks => solve iteratively.

    if (total_blocks < 10) return false;

    uint32_t sb_block = SB_BLOCK_INDEX;
    uint32_t root_blocks = ROOT_DIR_BLOCKS;

    // iterative solve for fat_blocks & num_data_blocks
    uint32_t fat_blocks = 1;
    uint64_t data_blocks = 0;
    for (int iter = 0; iter < 10; ++iter) {
        uint64_t reserved = 1 /*super*/ + fat_blocks + root_blocks;
        if (reserved >= total_blocks) return false;
        data_blocks = total_blocks - reserved;
        uint64_t fat_bytes = data_blocks * sizeof(uint32_t);
        uint32_t fat_blocks_new = (uint32_t)((fat_bytes + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE);
        if (fat_blocks_new == fat_blocks) break;
        fat_blocks = fat_blocks_new;
    }

    // prepare superblock
    fs_superblock_t sb;
    memset(&sb,0,sizeof(sb));
    sb.magic = FS_MAGIC;
    sb.version = 1;
    sb.total_blocks = total_blocks;
    sb.fat_start = sb_block + 1;
    sb.fat_blocks = fat_blocks;
    sb.root_start = sb.fat_start + sb.fat_blocks;
    sb.root_blocks = root_blocks;
    sb.data_start = sb.root_start + sb.root_blocks;

    // zero all FS blocks on disk area
    zero_blocks(disk_no, fs_start_block + sb_block, (uint32_t)total_blocks);

    // write superblock (first sector of block 0)
    if (!write_blocks(disk_no, fs_start_block + sb_block, 1, &sb)) return false;

    // init FAT with all free markers (0 means free)
    uint32_t *fat_buf = malloc((size_t)fat_blocks * FS_BLOCK_SIZE);
    if (!fat_buf) return false;
    for (uint64_t i = 0; i < data_blocks; ++i) fat_buf[i] = FAT_FREE;
    // pad remaining of last block
    uint64_t fat_entries = ((uint64_t)fat_blocks * FS_BLOCK_SIZE) / sizeof(uint32_t);
    for (uint64_t i = data_blocks; i < fat_entries; ++i) fat_buf[i] = FAT_FREE;

    if (!write_blocks(disk_no, fs_start_block + sb.fat_start, fat_blocks, fat_buf)) {
        free(fat_buf);
        return false;
    }
    free(fat_buf);

    // init root directory (array of fs_dir_entry_t) filled with zeros
    void *root_buf = malloc((size_t)root_blocks * FS_BLOCK_SIZE);
    if (!root_buf) return false;
    memset(root_buf, 0, (size_t)root_blocks * FS_BLOCK_SIZE);
    if (!write_blocks(disk_no, fs_start_block + sb.root_start, root_blocks, root_buf)) {
        free(root_buf);
        return false;
    }
    free(root_buf);

    // success
    return true;
}

/*
  mount FS located at given disk and starting block.
  On success loads FAT and root directory into memory.
*/
bool fs_mount(int disk_no, uint32_t fs_start_block) {
    // read superblock
    fs_superblock_t sb;
    if (!read_blocks(disk_no, fs_start_block + SB_BLOCK_INDEX, 1, &sb)) return false;
    if (sb.magic != FS_MAGIC) return false;

    // populate global state
    fs_disk = disk_no;
    fs_total_blocks = sb.total_blocks;
    fs_fat_start = sb.fat_start + fs_start_block;   // absolute disk block index
    fs_fat_blocks = sb.fat_blocks;
    fs_root_start = sb.root_start + fs_start_block;
    fs_root_blocks = sb.root_blocks;
    fs_data_start = sb.data_start + fs_start_block;

    // compute number of data blocks
    uint64_t reserved = 1 + fs_fat_blocks + fs_root_blocks;
    fs_num_data_blocks = (uint32_t)(fs_total_blocks - reserved);

    // load FAT into memory
    size_t fat_bytes = (size_t)fs_fat_blocks * FS_BLOCK_SIZE;
    fs_fat = malloc(fat_bytes);
    if (!fs_fat) return false;
    if (!read_blocks(fs_disk, fs_fat_start, fs_fat_blocks, fs_fat)) {
        free(fs_fat);
        fs_fat = NULL;
        return false;
    }

    // load root dir
    size_t root_bytes = (size_t)fs_root_blocks * FS_BLOCK_SIZE;
    fs_root = malloc(root_bytes);
    if (!fs_root) {
        free(fs_fat);
        fs_fat = NULL;
        return false;
    }
    if (!read_blocks(fs_disk, fs_root_start, fs_root_blocks, fs_root)) {
        free(fs_fat); fs_fat = NULL;
        free(fs_root); fs_root = NULL;
        return false;
    }

    // compute max entries
    fs_max_root_entries = (uint32_t)((fs_root_blocks * FS_BLOCK_SIZE) / sizeof(fs_dir_entry_t));
    if (fs_max_root_entries > MAX_FILES) fs_max_root_entries = MAX_FILES;

    return true;
}

/* persist FAT and root back to disk */
static bool fs_sync() {
    if (!fs_fat || !fs_root) return false;
    if (!write_blocks(fs_disk, fs_fat_start, fs_fat_blocks, fs_fat)) return false;
    if (!write_blocks(fs_disk, fs_root_start, fs_root_blocks, fs_root)) return false;
    return true;
}

/* ----------------- allocation helpers ----------------- */

static int find_free_fat_entry() {
    for (uint32_t i = 0; i < fs_num_data_blocks; ++i) {
        if (fs_fat[i] == FAT_FREE) return (int)i;
    }
    return -1;
}

/* allocate count contiguous blocks? Our simple alloc will allocate chain of single blocks */
static int allocate_blocks_chain(uint32_t blocks_needed, uint32_t *out_first) {
    if (blocks_needed == 0) return -1;
    int first = -1;
    int prev = -1;
    for (uint32_t i = 0; i < blocks_needed; ++i) {
        int idx = find_free_fat_entry();
        if (idx < 0) {
            // free the ones already allocated
            int cur = first;
            while (cur != -1) {
                int next = (int)fs_fat[cur];
                fs_fat[cur] = FAT_FREE;
                cur = next == FAT_EOC ? -1 : next;
            }
            return -1;
        }
        // mark allocated
        if (first == -1) first = idx;
        if (prev != -1) {
            fs_fat[prev] = (uint32_t)idx;
        }
        prev = idx;
        // tentatively mark EOC for now
        fs_fat[idx] = FAT_EOC;
    }
    *out_first = (uint32_t)first;
    return 0;
}

/* free chain */
static void free_chain(uint32_t first_index) {
    if (first_index >= fs_num_data_blocks) return;
    uint32_t cur = first_index;
    while (cur != FAT_EOC && cur < fs_num_data_blocks) {
        uint32_t next = fs_fat[cur];
        fs_fat[cur] = FAT_FREE;
        if (next == FAT_EOC) break;
        cur = next;
    }
}

/* ----------------- dir helpers ----------------- */

static int find_dir_entry(const char *name) {
    for (uint32_t i = 0; i < fs_max_root_entries; ++i) {
        if (fs_root[i].flags != 0) {
            if (strncmp(fs_root[i].name, name, FILENAME_MAX_LEN) == 0) return (int)i;
        }
    }
    return -1;
}

static int create_dir_entry(const char *name) {
    for (uint32_t i = 0; i < fs_max_root_entries; ++i) {
        if (fs_root[i].flags == 0) {
            memset(&fs_root[i], 0, sizeof(fs_dir_entry_t));
            strncpy(fs_root[i].name, name, FILENAME_MAX_LEN-1);
            fs_root[i].first_block = FAT_EOC; // no blocks yet
            fs_root[i].size = 0;
            fs_root[i].flags = 1; // in-use
            return (int)i;
        }
    }
    return -1;
}

/* ----------------- public file operations ----------------- */

/*
  Create file with given name (empty). Returns true on success.
*/
bool fs_create(const char *name) {
    if (!fs_root || !fs_fat) return false;
    if (find_dir_entry(name) >= 0) return false; // already exists
    int idx = create_dir_entry(name);
    if (idx < 0) return false;
    return fs_sync();
}

/*
  Write data to file (overwrite). Creates file if not exist.
  Returns true on success.
*/
bool fs_write_file(const char *name, const void *data, uint64_t size) {
    if (!fs_root || !fs_fat) return false;
    int diridx = find_dir_entry(name);
    if (diridx < 0) {
        diridx = create_dir_entry(name);
        if (diridx < 0) return false;
    }

    // free existing blocks
    uint32_t first_block = fs_root[diridx].first_block;
    if (first_block != FAT_EOC && first_block != 0) {
        free_chain(first_block);
    }

    // compute blocks needed
    uint32_t blocks_needed = (uint32_t)((size + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE);

    if (blocks_needed == 0) {
        // empty file => mark size 0, first_block = EOC
        fs_root[diridx].first_block = FAT_EOC;
        fs_root[diridx].size = 0;
        return fs_sync();
    }

    uint32_t new_first;
    if (allocate_blocks_chain(blocks_needed, &new_first) != 0) return false;

    // write data block by block following the chain
    uint32_t cur = new_first;
    uint64_t remaining = size;
    const uint8_t *src = (const uint8_t*)data;
    while (true) {
        // compute write size for this block
        uint32_t write_size = (uint32_t)(remaining > FS_BLOCK_SIZE ? FS_BLOCK_SIZE : remaining);

        // build a block buffer (zero pad last block)
        uint8_t *block_buf = malloc(FS_BLOCK_SIZE);
        if (!block_buf) return false;
        memset(block_buf, 0, FS_BLOCK_SIZE);
        if (write_size) memcpy(block_buf, src, write_size);

        // write to disk block: disk block index = data_start + cur
        if (!write_blocks(fs_disk, fs_data_start + cur, 1, block_buf)) {
            free(block_buf);
            return false;
        }
        free(block_buf);

        src += write_size;
        remaining -= write_size;

        if (fs_fat[cur] == FAT_EOC) break;
        cur = fs_fat[cur];
    }

    fs_root[diridx].first_block = new_first;
    fs_root[diridx].size = size;

    return fs_sync();
}

/*
  Read entire file into provided buffer. Returns bytes read, or -1 on error.
  Buffer should be large enough for file size (use fs_get_file_size).
*/
int64_t fs_read_file(const char *name, void *out_buf, uint64_t buf_size) {
    if (!fs_root || !fs_fat) return -1;
    int diridx = find_dir_entry(name);
    if (diridx < 0) return -1;
    uint64_t filesize = fs_root[diridx].size;
    if (buf_size < filesize) return -1;

    uint32_t cur = fs_root[diridx].first_block;
    if (cur == FAT_EOC) return 0; // empty file

    uint8_t *dst = (uint8_t*)out_buf;
    uint64_t remaining = filesize;
    while (true) {
        // read one block
        uint8_t *block_buf = malloc(FS_BLOCK_SIZE);
        if (!block_buf) return -1;
        if (!read_blocks(fs_disk, fs_data_start + cur, 1, block_buf)) { free(block_buf); return -1; }

        uint32_t to_copy = (uint32_t)(remaining > FS_BLOCK_SIZE ? FS_BLOCK_SIZE : remaining);
        if (to_copy) memcpy(dst, block_buf, to_copy);
        free(block_buf);

        dst += to_copy;
        remaining -= to_copy;
        if (remaining == 0) break;
        if (fs_fat[cur] == FAT_EOC) break;
        cur = fs_fat[cur];
    }
    return (int64_t)filesize;
}

/*
  Remove file
*/
bool fs_remove(const char *name) {
    int diridx = find_dir_entry(name);
    if (diridx < 0) return false;
    uint32_t first = fs_root[diridx].first_block;
    if (first != FAT_EOC && first != 0) free_chain(first);
    memset(&fs_root[diridx], 0, sizeof(fs_dir_entry_t));
    return fs_sync();
}

/* get file size, or -1 if not exist */
int64_t fs_get_file_size(const char *name) {
    int diridx = find_dir_entry(name);
    if (diridx < 0) return -1;
    return (int64_t)fs_root[diridx].size;
}


void fs_list_files(fs_list_cb cb, void *user) {
    if (!cb) return;
    for (uint32_t i = 0; i < fs_max_root_entries; ++i) {
        if (fs_root[i].flags != 0) {
            cb(fs_root[i].name, fs_root[i].size, user);
        }
    }
}

/* ----------------- unmount ----------------- */
void fs_unmount() {
    if (!fs_fat || !fs_root) return;
    fs_sync();
    free(fs_fat); fs_fat = NULL;
    free(fs_root); fs_root = NULL;
    fs_disk = -1;
}

/* ----------------- usage example (for reference) ----------------- */
void kfs_test(){
     // format 10000 blocks starting at block 0
     fs_format(1, 0, 10000);

     // mount
     fs_mount(1, 0);

     // create and write
     const char *txt = "Hello KeblaFS!";
     fs_write_file("hello.txt", txt, strlen(txt));

     // read back
     int64_t sz = fs_get_file_size("hello.txt");
     char *buf = malloc(sz+1);
     fs_read_file("hello.txt", buf, sz);
     buf[sz]=0;
     printf("buf: %s\n", buf);
     free(buf);

     // list
     void cb(const char *n, uint64_t s, void *u){ printf("%s (%llu bytes)\n", n, s); }
     fs_list_files(cb, NULL);

     fs_unmount();
}
















