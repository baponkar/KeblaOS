
#include "../memory/kheap.h"
#include "../bootloader/boot.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../memory/kheap.h"


#include "fs.h"

Superblock *sb;

// Initialize filesystem
bool fs_init(HBA_PORT_T *port) {
    sb = (Superblock *)(uintptr_t) kheap_alloc(sizeof(Superblock));

    if (!read_superblock(port, sb)) {
        printf("Failed to read superblock\n");
        return false;
    }
    
    if (sb->magic != FS_MAGIC) {
        printf("Filesystem not found, formatting...\n");
        if (!format_fs(port)) {
            printf("Format failed\n");
            return false;
        }
        if (!read_superblock(port, sb)) {
            return false;
        }
    }
    return true;
}

// Format filesystem
bool format_fs(HBA_PORT_T *port) {

    sb->magic = FS_MAGIC;
    sb->version = 1;
    sb->block_size = BLOCK_SIZE;
    sb->inode_count = INODE_COUNT;
    sb->inode_table_start = 1;
    sb->bitmap_start = 1 + ((INODE_COUNT * sizeof(Inode) + BLOCK_SIZE - 1) / BLOCK_SIZE);


    // Calculate total blocks using IDENTIFY (implementation omitted for brevity)
    // Assume total_blocks is known or retrieved via AHCI commands
    sb->total_blocks = 1000000; // Example value
    
    // Calculate bitmap blocks needed
    uint32_t data_blocks = sb->total_blocks - sb->bitmap_start;
    sb->bitmap_blocks = (data_blocks + BLOCK_SIZE * 8 - 1) / (BLOCK_SIZE * 8);
    sb->data_start = sb->bitmap_start + sb->bitmap_blocks;

    // Write superblock
    if (!ahci_write(port, 0, 0, 1, (uint16_t*)sb)) {
        return false;
    }

    // Initialize inode table
    uint16_t zero[BLOCK_SIZE/2] = {0};
    uint32_t inode_blocks = (INODE_COUNT * sizeof(Inode) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    for (uint32_t i = 0; i < inode_blocks; i++) {
        if (!ahci_write(port, sb->inode_table_start + i, 0, 1, zero)) {
            return false;
        }
    }

    // Initialize bitmap
    for (uint32_t i = 0; i < sb->bitmap_blocks; i++) {
        if (!ahci_write(port, sb->bitmap_start + i, 0, 1, zero)) {
            return false;
        }
    }

    printf("[INFO] Filesystem formated successfully.\n");
    return true;
}

// Read superblock
bool read_superblock(HBA_PORT_T *port, Superblock *sb) {
    return ahci_read(port, 0, 0, 1, (uint16_t*)sb);
}

// Allocate data block
uint32_t allocate_block(HBA_PORT_T *port) {
    Superblock sb;
    if (!read_superblock(port, &sb)) return 0;

    for (uint32_t i = 0; i < sb.bitmap_blocks; i++) {
        uint16_t block[BLOCK_SIZE/2];
        if (!ahci_read(port, sb.bitmap_start + i, 0, 1, block)) return 0;
        
        uint8_t *bytes = (uint8_t*)block;
        for (uint32_t j = 0; j < BLOCK_SIZE; j++) {
            if (bytes[j] != 0xFF) {
                for (int bit = 0; bit < 8; bit++) {
                    if (!(bytes[j] & (1 << bit))) {
                        bytes[j] |= (1 << bit);
                        ahci_write(port, sb.bitmap_start + i, 0, 1, block);
                        return sb.data_start + i * BLOCK_SIZE * 8 + j * 8 + bit;
                    }
                }
            }
        }
    }
    return 0; // No free blocks
}

// Read inode
bool get_inode(HBA_PORT_T *port, uint32_t inode_num, Inode *inode) {
    Superblock sb;
    if (!read_superblock(port, &sb) || inode_num >= sb.inode_count) return false;

    uint32_t block = sb.inode_table_start + (inode_num * sizeof(Inode)) / BLOCK_SIZE;
    uint16_t data[BLOCK_SIZE/2];
    if (!ahci_read(port, block, 0, 1, data)) return false;
    
    memcpy(inode, (uint8_t*)data + (inode_num * sizeof(Inode)) % BLOCK_SIZE, sizeof(Inode));
    return true;
}

// Write inode
bool put_inode(HBA_PORT_T *port, uint32_t inode_num, Inode *inode) {
    Superblock sb;
    if (!read_superblock(port, &sb) || inode_num >= sb.inode_count) return false;

    uint32_t block = sb.inode_table_start + (inode_num * sizeof(Inode)) / BLOCK_SIZE;
    uint16_t data[BLOCK_SIZE/2];
    if (!ahci_read(port, block, 0, 1, data)) return false;
    
    memcpy((uint8_t*)data + (inode_num * sizeof(Inode)) % BLOCK_SIZE, inode, sizeof(Inode));
    return ahci_write(port, block, 0, 1, data);
}

// Read file data
int read_file(HBA_PORT_T *port, uint32_t inode_num, uint8_t *buf, uint32_t size, uint32_t offset) {
    Inode inode;
    if (!get_inode(port, inode_num, &inode)) return -1;

    uint32_t bytes_read = 0;
    uint32_t current_block = offset / BLOCK_SIZE;
    uint32_t block_offset = offset % BLOCK_SIZE;

    while (bytes_read < size && current_block < (inode.size + BLOCK_SIZE - 1) / BLOCK_SIZE) {
        uint32_t phys_block;
        if (current_block < DIRECT_BLOCKS) {
            phys_block = inode.blocks[current_block];
        } else {
            // Handle indirect blocks
            uint32_t indirect_idx = current_block - DIRECT_BLOCKS;
            uint32_t indirect[INDIRECT_BLOCKS];
            if (!inode.indirect_block || !ahci_read(port, inode.indirect_block, 0, 1, (uint16_t*)indirect)) {
                break;
            }
            phys_block = indirect[indirect_idx];
        }

        if (!phys_block) break;

        uint16_t block_data[BLOCK_SIZE/2];
        if (!ahci_read(port, phys_block, 0, 1, block_data)) break;

        uint32_t copy_size = BLOCK_SIZE - block_offset;
        if (copy_size > size - bytes_read) copy_size = size - bytes_read;

        memcpy(buf + bytes_read, (uint8_t*)block_data + block_offset, copy_size);
        bytes_read += copy_size;
        block_offset = 0;
        current_block++;
    }
    return bytes_read;
}

// Create file (simplified)
bool create_file(HBA_PORT_T *port, uint32_t parent_inode, const char *name, uint32_t *new_inode) {
    // Find free inode
    // Superblock sb;

    if (!read_superblock(port, sb)) return false;

    Inode inode;
    for (uint32_t i = 1; i < sb->inode_count; i++) { // Skip inode 0
        if (get_inode(port, i, &inode) && inode.mode == 0) {
            // Initialize new inode
            inode.mode = 0x8000; // Regular file
            inode.size = 0;
            memset(inode.blocks, 0, sizeof(inode.blocks));
            inode.indirect_block = 0;
            if (!put_inode(port, i, &inode)) return false;
            
            // Add directory entry
            DirEntry entry;
            strncpy(entry.name, name, sizeof(entry.name));
            entry.inode_num = i;
            
            // Append to parent directory (implementation omitted)
            // ...
            
            *new_inode = i;
            return true;
        }
    }
    return false;
}



// Write data to file
int write_file(HBA_PORT_T* port, uint32_t inode_num, uint8_t* buf, uint32_t size, uint32_t offset) {
        Inode inode;
        if (!get_inode(port, inode_num, &inode)) return -1;

        uint32_t total_size = offset + size;
        uint32_t blocks_needed = (total_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        uint32_t current_blocks = (inode.size + BLOCK_SIZE - 1) / BLOCK_SIZE;

        // Allocate new blocks if needed
        for (uint32_t i = current_blocks; i < blocks_needed; i++) {
            uint32_t block = allocate_block(port);
            if (!block) return -1;

            if (i < DIRECT_BLOCKS) {
                inode.blocks[i] = block;
            } else {
                // Handle indirect blocks
                if (!inode.indirect_block) {
                    inode.indirect_block = allocate_block(port);
                    if (!inode.indirect_block) return -1;
                    // Initialize indirect block with zeros
                    uint16_t zero[BLOCK_SIZE/2] = {0};
                    ahci_write(port, inode.indirect_block, 0, 1, zero);
                }
        
                uint32_t indirect[INDIRECT_BLOCKS];
                ahci_read(port, inode.indirect_block, 0, 1, (uint16_t*)indirect);
                indirect[i - DIRECT_BLOCKS] = block;
                ahci_write(port, inode.indirect_block, 0, 1, (uint16_t*)indirect);
            }
        }

        // Write data block by block
        uint32_t bytes_written = 0;
        uint32_t block_idx = offset / BLOCK_SIZE;
        uint32_t block_offset = offset % BLOCK_SIZE;

        while (bytes_written < size) {
            uint32_t phys_block;
            if (block_idx < DIRECT_BLOCKS) {
                phys_block = inode.blocks[block_idx];
            } else {
                uint32_t indirect[INDIRECT_BLOCKS];
                ahci_read(port, inode.indirect_block, 0, 1, (uint16_t*)indirect);
                phys_block = indirect[block_idx - DIRECT_BLOCKS];
            }

        if (!phys_block) return -1;

        // Handle partial block writes
        uint8_t block_data[BLOCK_SIZE];
        uint32_t write_size = BLOCK_SIZE - block_offset;
        if (write_size > size - bytes_written)
            write_size = size - bytes_written;

        // Read existing block if not overwriting completely
        if (block_offset != 0 || write_size != BLOCK_SIZE)
            ahci_read(port, phys_block, 0, 1, (uint16_t*)block_data);

        memcpy(block_data + block_offset, buf + bytes_written, write_size);
        ahci_write(port, phys_block, 0, 1, (uint16_t*)block_data);

        bytes_written += write_size;
        block_idx++;
        block_offset = 0;
    }

    // Update file size
    if (total_size > inode.size) {
        inode.size = total_size;
        put_inode(port, inode_num, &inode);
    }

        return bytes_written;
    }


// List directory contents
bool list_directory(HBA_PORT_T* port, uint32_t inode_num) {
    Inode dir_inode;
    if (!get_inode(port, inode_num, &dir_inode) || !(dir_inode.mode & 0x4000)) {  // Check directory flag
        printf("Not a directory\n");
        return false;
    }

    uint32_t blocks_to_read = (dir_inode.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    printf("Directory listing (inode %d):\n", inode_num);

    for (uint32_t block_idx = 0; block_idx < blocks_to_read; block_idx++) {
        uint32_t phys_block;
        if (block_idx < DIRECT_BLOCKS) {
            phys_block = dir_inode.blocks[block_idx];
        } else {
            if (!dir_inode.indirect_block) break;
            uint32_t indirect[INDIRECT_BLOCKS];
            ahci_read(port, dir_inode.indirect_block, 0, 1, (uint16_t*)indirect);
            phys_block = indirect[block_idx - DIRECT_BLOCKS];
        }

        if (!phys_block) continue;

        DirEntry entries[BLOCK_SIZE / sizeof(DirEntry)];
        ahci_read(port, phys_block, 0, 1, (uint16_t*)entries);

        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(DirEntry); i++) {
            if (entries[i].inode_num != 0 && entries[i].name[0] != '\0') {
                printf("  %-28s [inode %d]\n",  entries[i].name, entries[i].inode_num);
            }
        }
    }
    return true;
}



void ahci_test(HBA_MEM_T *abar) {
    int port_no = probePort(abar);
    if(port_no == -1){
        return;
    }
    // 1. Initialize AHCI port
    HBA_PORT_T *port = (HBA_PORT_T *) &abar->ports[port_no];
    portRebase(port, port_no);
    
    // 2. Initialize filesystem
    if (!fs_init(port)) {
        printf("Filesystem initialization failed!\n");
        return;
    }

    // 3. Create new file
    uint32_t file_inode;
    if (!create_file(port, 0, "test.txt", &file_inode)) {
        printf("File creation failed\n");
        return;
    }

    // // 4. Write data
    // const char *data = "File system test data";
    // write_file(port, file_inode, (uint8_t*)data, strlen(data), 0);

    // // 5. Read data
    // uint8_t buf[512];
    // int bytes = read_file(port, file_inode, buf, 512, 0);
    // printf("Read %d bytes: %.*s\n", bytes, bytes, buf);

    // // 6. List directory
    // list_directory(port, 0);
}