
/*
Second Extended Filesystem (ext2fs)

📦 ext2 On-Disk Structure
+--------------------+ 0 KB
| Boot Sector        |
| (first 1024 bytes) |
+--------------------+ 1 KB
| Superblock         |
+--------------------+ ~2 KB
| Group Descriptors  |
+--------------------+
| Block/Inode Bitmap |
+--------------------+
| Inode Table        |
+--------------------+
| Data Blocks        |
+--------------------+


References:
    https://www.nongnu.org/ext2-doc/ext2.html
    https://wiki.osdev.org/Ext2
    https://www.geeksforgeeks.org/dsa/little-and-big-endian-mystery/
*/

#include "../../lib/string.h"
#include "../../lib/stdio.h"
#include "../../memory/vmm.h"
#include "../../memory/kheap.h"
#include "../../driver/disk/ahci/ahci.h"

#include "ext2.h"

#define PARTITION_LBA_OFFSET 2048

#define SECTOR_SIZE 512
#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPERBLOCK_SECTOR (EXT2_SUPERBLOCK_OFFSET / SECTOR_SIZE)

#define EXT2_S_IFMASK 0xF000
#define EXT2_S_IFDIR  0x4000  // Directory
#define EXT2_S_IFREG  0x8000  // Regular file


extern HBA_PORT_T *port;
static ext2_info_t fs;

static void read_disk(uint32_t lba, uint32_t count, void *buf) {
    ahci_read(port, PARTITION_LBA_OFFSET + lba, 0, count, buf);
}

bool ext2_init() {

    if (!port) return false;

    ext2_read_superblock();
    return true;
}

void ext2_read_superblock() {
    uint8_t *buf = (uint8_t *) kheap_alloc(1024, ALLOCATE_DATA);

    read_disk(2, 2, (void *)buf); // Read sector 2 (LBA=2) → 1024 bytes

    fs.inodes_count = *(uint32_t *)(buf + 0);
    fs.blocks_count = *(uint32_t *)(buf + 4);
    uint32_t log_block_size = *(uint32_t *)(buf + 24);
    fs.block_size = 1024 << log_block_size;
    fs.inodes_per_group = *(uint32_t *)(buf + 40);
    fs.blocks_per_group = *(uint32_t *)(buf + 32);
    fs.first_inode = *(uint32_t *)(buf + 84);
    fs.inode_size = *(uint16_t *)(buf + 88);

    printf("EXT2: %d blocks, %d inodes, block size: %d\n", fs.blocks_count, fs.inodes_count, fs.block_size);

    if (fs.block_size == 0 || fs.inode_size == 0) {
        printf("EXT2: Invalid superblock values\n");
    }

    kheap_free(buf, 1024);
}


static void ext2_read_block(uint32_t block_num, void *buffer) {
    uint32_t lba = (block_num * fs.block_size) / SECTOR_SIZE;
    uint32_t sectors = fs.block_size / SECTOR_SIZE;
    ahci_read(port, PARTITION_LBA_OFFSET + lba, 0, sectors, buffer);
}

ext2_inode_t ext2_read_inode(uint32_t inode_id) {
    inode_id--;  // ext2 inodes start at 1

    uint32_t group = inode_id / fs.inodes_per_group;
    uint32_t index = inode_id % fs.inodes_per_group;

    // Read Group Descriptor
    uint32_t gd_block = (fs.block_size == 1024) ? 2 : 1;
    void *gd_buf = (void *) kheap_alloc(fs.block_size, ALLOCATE_DATA);
    ext2_read_block(gd_block, gd_buf);

    uint32_t inode_table_block = *(uint32_t *)(gd_buf + group * 32 + 8); // offset 8 in each descriptor
    kheap_free(gd_buf, fs.block_size);

    // Read the specific inode
    void *inode_block = (void *)kheap_alloc(fs.inode_size, ALLOCATE_DATA);
    uint32_t inodes_per_block = fs.block_size / fs.inode_size;
    uint32_t block_offset = index / inodes_per_block;
    uint32_t offset_in_block = index % inodes_per_block;

    ext2_read_block(inode_table_block + block_offset, inode_block);

    ext2_inode_t inode;
    memcpy(&inode, inode_block + offset_in_block * fs.inode_size, sizeof(ext2_inode_t));
    kheap_free(inode_block, fs.inode_size);

    return inode;
}

static int find_free_bit(uint8_t *bitmap, uint32_t size) {
    for (uint32_t byte = 0; byte < size; byte++) {
        if (bitmap[byte] != 0xFF) {
            for (int bit = 0; bit < 8; bit++) {
                if (!(bitmap[byte] & (1 << bit))) {
                    return byte * 8 + bit;
                }
            }
        }
    }
    return -1;
}


static uint32_t ext2_allocate_inode() {
    void *gd_buf = kheap_alloc(fs.block_size, ALLOCATE_DATA);
    ext2_read_block((fs.block_size == 1024) ? 2 : 1, gd_buf);

    uint32_t inode_bitmap_block = *(uint32_t *)(gd_buf + 0);
    kheap_free(gd_buf, fs.block_size);

    void *bitmap = (void *)kheap_alloc(fs.block_size, ALLOCATE_DATA);
    char *bitmap_data = (char *) bitmap;
    ext2_read_block(inode_bitmap_block, bitmap);

    int inode_index = find_free_bit(bitmap, fs.block_size);
    if (inode_index == -1) return 0;

    bitmap_data[inode_index / 8] |= (1 << (inode_index % 8));
    // Write updated bitmap back
    ahci_write(port, PARTITION_LBA_OFFSET + (inode_bitmap_block * fs.block_size / SECTOR_SIZE), 0, fs.block_size / 512, bitmap);

    kheap_free(bitmap, fs.block_size);

    return inode_index + 1; // Inode numbers are 1-based
}

static void ext2_create_inode(uint32_t inode_id, uint16_t mode, uint32_t block_ptr) {
    uint32_t group = (inode_id - 1) / fs.inodes_per_group;
    uint32_t index = (inode_id - 1) % fs.inodes_per_group;

    void *gd_buf = kheap_alloc(fs.block_size, ALLOCATE_DATA);
    ext2_read_block((fs.block_size == 1024) ? 2 : 1, gd_buf);
    uint32_t inode_table_block = *(uint32_t *)(gd_buf + group * 32 + 8);
    kheap_free(gd_buf, fs.block_size);

    uint32_t inodes_per_block = fs.block_size / fs.inode_size;
    uint32_t block_offset = index / inodes_per_block;
    uint32_t offset_in_block = index % inodes_per_block;

    void *block_buf = kheap_alloc(fs.block_size, ALLOCATE_DATA);
    ext2_read_block(inode_table_block + block_offset, block_buf);

    ext2_inode_t *inode = (ext2_inode_t *)(block_buf + offset_in_block * fs.inode_size);
    memset(inode, 0, sizeof(ext2_inode_t));
    inode->mode = mode;
    inode->size = fs.block_size;
    inode->block[0] = block_ptr;
    inode->blocks = fs.block_size / 512;

    ahci_write(port, PARTITION_LBA_OFFSET + ((inode_table_block + block_offset) * fs.block_size / 512), 0, fs.block_size / 512, block_buf);
    kheap_free(block_buf, fs.block_size);
}


static void ext2_add_dir_entry(uint32_t parent_inode_id, uint32_t new_inode_id, const char *name, uint8_t file_type) {
    ext2_inode_t parent = ext2_read_inode(parent_inode_id);
    void *block = kheap_alloc(fs.block_size, ALLOCATE_DATA);
    ext2_read_block(parent.block[0], block);

    uint32_t offset = 0;
    while (offset < fs.block_size) {
        ext2_dir_entry_t *entry = (ext2_dir_entry_t *)(block + offset);
        if (offset + entry->rec_len >= fs.block_size) {
            uint16_t used_len = 8 + entry->name_len;
            used_len = (used_len + 3) & ~3;

            uint16_t remaining = entry->rec_len - used_len;

            if (remaining >= (8 + strlen(name))) {
                entry->rec_len = used_len;

                ext2_dir_entry_t *new_entry = (ext2_dir_entry_t *)(block + offset + used_len);
                new_entry->inode = new_inode_id;
                new_entry->name_len = strlen(name);
                new_entry->rec_len = remaining;
                new_entry->file_type = file_type;
                memcpy(new_entry->name, name, new_entry->name_len);

                ahci_write(port, PARTITION_LBA_OFFSET + (parent.block[0] * fs.block_size / SECTOR_SIZE), 0, fs.block_size / 512, block);
                break;
            }
        }

        offset += entry->rec_len;
    }

    kheap_free(block, fs.block_size);
}




void ext2_list_dir(uint32_t inode_id) {
    ext2_inode_t inode = ext2_read_inode(inode_id);

    if ((inode.mode & EXT2_S_IFMASK) != EXT2_S_IFDIR){  // not a directory
        printf("Not a directory\n");
        return;
    }

    void *block = (void *) kheap_alloc(fs.block_size, ALLOCATE_DATA);

    for (int i = 0; i < 12 && inode.block[i]; i++) {
        ext2_read_block(inode.block[i], block);

        uint32_t offset = 0;
        while (offset < fs.block_size) {
            ext2_dir_entry_t *entry = (ext2_dir_entry_t *)(block + offset);
            if (entry->inode == 0) break;

            char name[256] = {0};
            memcpy(name, entry->name, entry->name_len);
            name[entry->name_len] = '\0';

            printf("[inode %d] %s\n", entry->inode, name);

            offset += entry->rec_len;
        }
    }

    kheap_free(block, fs.block_size);
}

void ext2_create_file(uint32_t parent_inode_id, const char *name) {
    uint32_t inode_id = ext2_allocate_inode();
    if (!inode_id) {
        printf("EXT2: No free inode\n");
        return;
    }

    ext2_create_inode(inode_id, EXT2_S_IFREG | 0x1FF, 0); // 0777
    ext2_add_dir_entry(parent_inode_id, inode_id, name, 1); // EXT2_FT_REG_FILE
}

void ext2_create_dir(uint32_t parent_inode_id, const char *name) {
    uint32_t inode_id = ext2_allocate_inode();
    if (!inode_id) {
        printf("EXT2: No free inode\n");
        return;
    }

    uint32_t block_ptr = /* Allocate block like inode bitmap */ 0; // You must implement ext2_allocate_block
    ext2_create_inode(inode_id, EXT2_S_IFDIR | 0x1FF, block_ptr);

    void *block = kheap_alloc(fs.block_size, ALLOCATE_DATA);
    memset(block, 0, fs.block_size);

    ext2_dir_entry_t *dot = (ext2_dir_entry_t *)block;
    dot->inode = inode_id;
    dot->name_len = 1;
    dot->rec_len = 12;
    dot->file_type = 2;
    strcpy(dot->name, ".");

    ext2_dir_entry_t *dotdot = (ext2_dir_entry_t *)((uint8_t *)block + 12);
    dotdot->inode = parent_inode_id;
    dotdot->name_len = 2;
    dotdot->rec_len = fs.block_size - 12;
    dotdot->file_type = 2;
    strcpy(dotdot->name, "..");

    ahci_write(port, PARTITION_LBA_OFFSET + (block_ptr * fs.block_size / SECTOR_SIZE), 0, fs.block_size / 512, block);
    kheap_free(block, fs.block_size);

    ext2_add_dir_entry(parent_inode_id, inode_id, name, 2); // EXT2_FT_DIR
}


void ext2_read_file(uint32_t inode_id) {
    ext2_inode_t inode = ext2_read_inode(inode_id);

    if ((inode.mode & EXT2_S_IFMASK) != EXT2_S_IFREG){  // not a regular file
        printf("Not a regular file\n");
        return;
    }

    uint32_t bytes_read = 0;
    void *buf = (void *) kheap_alloc(fs.block_size, ALLOCATE_DATA);
    char *data = (char *)buf;

    for (int i = 0; i < 12 && inode.block[i]; i++) {
        ext2_read_block(inode.block[i], buf);

        uint32_t bytes_to_read = fs.block_size;
        if (bytes_read + fs.block_size > inode.size) {
            bytes_to_read = inode.size - bytes_read;
        }

        for (uint32_t j = 0; j < bytes_to_read; j++) {
            putc(data[j]);
        }

        bytes_read += bytes_to_read;
    }

    kheap_free(buf, fs.block_size);
}



void ext2_test(){
    ext2_init();
    ext2_list_dir(2);         // List root directory
    ext2_read_file(12);       // Print testfile.txt content of inode 12
    ext2_list_dir(40961);     // List a subdirectory
    ext2_read_file(40962);     // Print nested.txt file content of 49154 

    // ext2_create_file(2, "myfile.txt");  // Creating myfile.txt file inside of rootdir
    // ext2_create_dir(2, "mydir");        // Creating a subdirectory in rootdir  
}