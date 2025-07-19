

#include "../../lib/string.h"
#include "../../lib/stdio.h"
#include "../../memory/vmm.h"
#include "../../memory/kheap.h"
#include "../../driver/disk/ahci/ahci.h"

#include "ext2.h"
#include "ext2_1.h"


#define PARTITION_LBA_OFFSET 2048

#define SECTOR_SIZE 512
#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPERBLOCK_SECTOR (EXT2_SUPERBLOCK_OFFSET / SECTOR_SIZE)

// Types Flags
#define EXT2_S_IFMASK 0xF000            // MASK
#define EXT2_S_FIFO          0x1000     // FIFO
#define EXT2_S_CHARACTER_DEVICE 0X2000  // Character Device
#define EXT2_S_IFDIR  0x4000            // Directory
#define EXT2_S_BLOCK_DEVICE 0x6000      // Block Device
#define EXT2_S_IFREG  0x8000            // Regular file
#define EXT2_S_SYMBOLIC_LINK 0xA000     // Symbolic Link
#define EXT2_S_UNIX_SOCKET 0xC000       // Unix Socket

#define EXT2_NAME_LEN 255


extern HBA_PORT_T *port;
ext2_superblock_t *superblock;
ext2_block_group_descriptor_t *bgd;



static void read_disk(uint32_t lba, uint32_t count, void *buf) {
    ahci_read(port, PARTITION_LBA_OFFSET + lba, 0, count, buf);
}

void read_superblock() {
    // Allocate memory for superblock
    superblock = (ext2_superblock_t *)kheap_alloc(sizeof(ext2_superblock_t), ALLOCATE_DATA);
    if (!superblock) {
        printf("[EXT2] Superblock allocation failed!\n");
        return;
    }

    // Allocate 1024-byte buffer to hold the superblock from disk (LBA 2 → offset 1024)
    uint8_t *buffer = (uint8_t *)kheap_alloc(1024, ALLOCATE_DATA);
    if (!buffer) {
        printf("[EXT2] Buffer allocation failed!\n");
        return;
    }

    // Read sector 2 (LBA 2) = 1024 bytes from disk into buffer
    read_disk(2, 2, buffer);

    // Copy entire 1024-byte buffer directly into the superblock structure
    memcpy(superblock, buffer, sizeof(ext2_superblock_t));

    // Calculate and print interpreted values
    uint32_t block_size = 1024 << superblock->s_log_block_size;
    printf("EXT2: %d blocks, %d inodes, block size: %d\n", 
           superblock->s_blocks_count,
           superblock->s_inodes_count,
           block_size);

    if (block_size == 0 || superblock->s_inode_size == 0) {
        printf("EXT2: Invalid superblock values\n");
    }

    // Done
    kheap_free(buffer, 1024);
}

static void read_group_descriptor_table(){
    
    // Allocate space for one block group descriptor (32 bytes)
    bgd = (ext2_block_group_descriptor_t *)kheap_alloc(sizeof(ext2_block_group_descriptor_t), ALLOCATE_DATA);
    if (!bgd) {
        printf("[EXT2] Block group descriptor allocation failed!\n");
        return;
    }

    // Allocate 1024-byte buffer to hold the superblock from disk (LBA 2 → offset 1024)
    uint8_t *buffer = (uint8_t *)kheap_alloc(1024, ALLOCATE_DATA);
    if (!buffer) {
        printf("[EXT2] Buffer allocation failed!\n");
        return;
    }

    // Read group descriptor table — it starts immediately after superblock
    // Superblock is at offset 1024 (LBA 2), so for 4K block size, BGD is in block 1 (offset 2048)
    // But for block size > 1024, both SB and BGD are in block 0
    // For simplicity, read 1 block at `block_num = (block_size == 1024 ? 2 : 1)`
    uint32_t bgd_lba = (1024 << superblock->s_log_block_size == 1024) ? 3 : 2;
    read_disk(bgd_lba, 2, buffer);

    // Copy first block group descriptor (32 bytes) from buffer
    memcpy(bgd, buffer, sizeof(ext2_block_group_descriptor_t));

    // Done
    kheap_free(buffer, 1024);
}

static void read_block(uint32_t block_num, void *buffer) {
    uint32_t lba = (block_num * 1024 << superblock->s_log_block_size) / SECTOR_SIZE;
    uint32_t sectors = 1024 << superblock->s_log_block_size / SECTOR_SIZE;
    ahci_read(port, PARTITION_LBA_OFFSET + lba, 0, sectors, buffer);
}

ext2_inode_t read_inode(uint32_t inode_id) {
    inode_id--;  // ext2 inodes start at 1

    uint32_t group = inode_id / superblock->s_inodes_per_group;
    uint32_t index = inode_id % superblock->s_inodes_per_group;

    uint32_t inode_table_block = bgd->bg_inode_table;

    // Read the specific inode
    void *inode_block = (void *)kheap_alloc(superblock->s_inode_size, ALLOCATE_DATA);
    uint32_t inodes_per_block = (1024 << superblock->s_log_block_size) / superblock->s_inode_size;
    uint32_t block_offset = index / inodes_per_block;
    uint32_t offset_in_block = index % inodes_per_block;

    read_block(inode_table_block + block_offset, inode_block);

    ext2_inode_t inode;
    memcpy(&inode, inode_block + offset_in_block * superblock->s_inode_size, sizeof(ext2_inode_t));
    kheap_free(inode_block, superblock->s_inode_size);

    return inode;
}

bool init(){
    if (!port){
        printf("[EXT2] : AHCI port is null\n");
        return false;
    }
    read_superblock();
    read_group_descriptor_table();

    printf("[EXT2] Successfully ext2 filesystem initialized\n");
    return true;
}

static void add_dir_entry(uint32_t parent_inode_id, uint32_t new_inode_id, const char *name, uint8_t file_type) {
    ext2_inode_t parent = read_inode(parent_inode_id);
    void *block = kheap_alloc(1024 << superblock->s_log_block_size, ALLOCATE_DATA);
    read_block(parent.block[0], block);

    uint32_t offset = 0;
    while (offset < 1024 << superblock->s_log_block_size) {
        ext2_dir_entry_t *entry = (ext2_dir_entry_t *)(block + offset);
        if (offset + entry->rec_len >= 1024 << superblock->s_log_block_size) {
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

                ahci_write(port, PARTITION_LBA_OFFSET + (parent.block[0] * (1024 << superblock->s_log_block_size) / SECTOR_SIZE), 0, (1024 << superblock->s_log_block_size) / 512, block);
                break;
            }
        }

        offset += entry->rec_len;
    }

    kheap_free(block, 1024 << superblock->s_log_block_size);
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


void list_dir(uint32_t inode_id) {
    ext2_inode_t inode = read_inode(inode_id);

    if ((inode.mode & EXT2_S_IFMASK) != EXT2_S_IFDIR){  // not a directory
        printf("Not a directory\n");
        return;
    }

    void *block = (void *) kheap_alloc(1024 << superblock->s_log_block_size, ALLOCATE_DATA);

    for (int i = 0; i < 12 && inode.block[i]; i++) {
        read_block(inode.block[i], block);

        uint32_t offset = 0;
        while (offset < 1024 << superblock->s_log_block_size) {
            ext2_dir_entry_t *entry = (ext2_dir_entry_t *)(block + offset);
            if (entry->inode == 0) break;

            char name[256] = {0};
            memcpy(name, entry->name, entry->name_len);
            name[entry->name_len] = '\0';

            printf("[inode %d] %s\n", entry->inode, name);

            offset += entry->rec_len;
        }
    }

    kheap_free(block, 1024 << superblock->s_log_block_size);
}

void read_file(uint32_t inode_id) {
    ext2_inode_t inode = read_inode(inode_id);

    if ((inode.mode & EXT2_S_IFMASK) != EXT2_S_IFREG){  // not a regular file
        printf("Not a regular file\n");
        return;
    }

    uint32_t bytes_read = 0;
    void *buf = (void *) kheap_alloc(1024 << superblock->s_log_block_size, ALLOCATE_DATA);
    char *data = (char *)buf;

    for (int i = 0; i < 12 && inode.block[i]; i++) {
        read_block(inode.block[i], buf);

        uint32_t bytes_to_read = 1024 << superblock->s_log_block_size;
        if (bytes_read + 1024 << superblock->s_log_block_size > inode.size) {
            bytes_to_read = inode.size - bytes_read;
        }

        for (uint32_t j = 0; j < bytes_to_read; j++) {
            putc(data[j]);
        }

        bytes_read += bytes_to_read;
    }

    kheap_free(buf, 1024 << superblock->s_log_block_size);
}


static uint32_t find_entry_in_dir(uint32_t parent_inode_id, const char *name) {
    ext2_inode_t parent = read_inode(parent_inode_id);

    if ((parent.mode & EXT2_S_IFMASK) != EXT2_S_IFDIR) {
        printf("EXT2: Parent inode %d is not a directory.\n", parent_inode_id);
        return 0; // Not a directory
    }

    void *block = (void *)kheap_alloc(1024 << superblock->s_log_block_size, ALLOCATE_DATA);
    if (!block) {
        printf("EXT2: Failed to allocate block for directory search\n");
        return 0;
    }

    // Iterate through direct blocks of the directory inode
    for (int i = 0; i < 12 && parent.block[i]; i++) {
       read_block(parent.block[i], block);

        uint32_t offset = 0;
        while (offset < 1024 << superblock->s_log_block_size) {
            ext2_dir_entry_t *entry = (ext2_dir_entry_t *)(block + offset);
            if (entry->inode == 0) { // End of valid entries in this block
                break;
            }

            // Extract the name from the directory entry
            char entry_name[EXT2_NAME_LEN + 1]; // Assuming EXT2_NAME_LEN is max name length (255)
            if (entry->name_len > EXT2_NAME_LEN) {
                entry->name_len = EXT2_NAME_LEN; // Prevent buffer overflow
            }
            memcpy(entry_name, entry->name, entry->name_len);
            entry_name[entry->name_len] = '\0'; // Null-terminate

            // Compare names
            if (strcmp(entry_name, name) == 0) {
                uint32_t found_inode = entry->inode;
                kheap_free(block, 1024 << superblock->s_log_block_size);
                return found_inode;
            }

            offset += entry->rec_len;
        }
    }

    kheap_free(block, 1024 << superblock->s_log_block_size);
    return 0; // Not found
}

uint32_t path_to_inode(const char *path) {
    if (!path || path[0] != '/') {
        printf("EXT2: Invalid path: %s\n", path);
        return 0;
    }

    // Start from root directory (inode 2 in ext2)
    uint32_t current_inode = 2;

    // Skip leading '/'
    const char *p = path;
    while (*p == '/') p++;

    char component[EXT2_NAME_LEN + 1];

    while (*p) {
        // Extract next component
        int len = 0;
        while (*p && *p != '/') {
            if (len < EXT2_NAME_LEN) {
                component[len++] = *p;
            }
            p++;
        }
        component[len] = '\0';

        // Skip multiple slashes
        while (*p == '/') p++;

        if (len > 0) {
            // Find this component in the current directory
            uint32_t next_inode = find_entry_in_dir(current_inode, component);
            if (!next_inode) {
                printf("EXT2: Path component '%s' not found under inode %d\n", component, current_inode);
                return 0; // Not found
            }
            current_inode = next_inode;
        }
    }

    return current_inode;
}



void ext2_test_1(){
    init();

    list_dir(2);         // List root directory

    read_file(12);       // Print testfile.txt content of inode 12

    list_dir(24577);     // List a subdirectory
    
    read_file(24578);     // Print nested.txt file content of 49154 

    const char *path = "/subdir/nested.txt";
    uint32_t inode_id = ext2_path_to_inode(path);
    if (inode_id) {
        printf("EXT2: Inode for %s is %d\n", path, inode_id);
        read_file(inode_id);
    } else {
        printf("EXT2: Path not found: %s\n", path);
    }
 
}
