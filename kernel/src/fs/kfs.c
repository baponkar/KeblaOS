
/*
Kebla File System (KFS)

*/

#include "../lib/stdio.h"
#include "../lib/string.h"

#include "kfs.h"


static bool read_block(HBA_PORT_T* port, uint32_t lba, void* buf) {
    return ahci_read(port, lba, 0, 1, (uint16_t*)buf);
}

static bool write_block(HBA_PORT_T* port, uint32_t lba, void* buf) {
    return ahci_write(port, lba, 0, 1, (uint16_t*)buf);
}

bool kfs_init(HBA_PORT_T* port, uint32_t total_blocks) {
    Superblock sb = {
        .magic = 0x4B465331, // 'KFS1'
        .total_blocks = total_blocks,
        .free_block = DATA_START
    };

    // Write superblock
    if (!write_block(port, 0, &sb)) return false;

    // Clear directory blocks
    char zero_block[BLOCK_SIZE] = {0};
    for (uint32_t i = 1; i <= DIR_BLOCKS; i++) {
        if (!write_block(port, i, zero_block)) return false;
    }

    return true;
}

bool kfs_create(HBA_PORT_T* port, const char* name, const void* data, uint32_t size) {
    printf("[KFS] Creating file: %s, size=%x\n", name, size);

    Superblock sb;
    if (!read_block(port, 0, &sb)) return false;
    if (sb.magic != 0x4B465331) {
        printf("[KFS] Invalid magic: 0x%x\n", sb.magic);
        return false;
    }

    DirEntry entries[MAX_FILES];
    for (int i = 0; i < DIR_BLOCKS; i++) {
        if (!read_block(port, 1 + i, ((char*)entries) + (i * BLOCK_SIZE))) return false;
    }

    int free_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!entries[i].used) {
            free_index = i;
            break;
        }
    }

    if (free_index == -1) {
        printf("[KFS] No free directory entry!\n");
        return false;
    }

    uint32_t blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (sb.free_block + blocks_needed > sb.total_blocks) return false;

    // Write file data
    const char* src = (const char*)data;
    for (uint32_t i = 0; i < blocks_needed; i++) {
        char temp[BLOCK_SIZE] = {0};
        for (int j = 0; j < BLOCK_SIZE && (i * BLOCK_SIZE + j) < size; j++)
            temp[j] = src[i * BLOCK_SIZE + j];

        if (!write_block(port, sb.free_block + i, temp)) return false;
    }

    // Update directory
    DirEntry* e = &entries[free_index];
    e->used = 1;
    e->start_block = sb.free_block;
    e->size_bytes = size;
    for (int i = 0; i < 100; i++) e->name[i] = name[i];
    
    sb.free_block += blocks_needed;

    // Write directory and superblock
    for (int i = 0; i < DIR_BLOCKS; i++) {
        if (!write_block(port, 1 + i, ((char*)entries) + (i * BLOCK_SIZE))) return false;
    }
    if (!write_block(port, 0, &sb)) return false;

    return true;
}


int kfs_read(HBA_PORT_T* port, const char* name, void* out_buf) {
    DirEntry entries[MAX_FILES];
    for (int i = 0; i < DIR_BLOCKS; i++) {
        if (!read_block(port, 1 + i, ((char*)entries) + (i * BLOCK_SIZE))) return -1;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && !strncmp(entries[i].name, name, 100)) {
            uint32_t blocks = (entries[i].size_bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
            for (uint32_t j = 0; j < blocks; j++) {
                if (!read_block(port, entries[i].start_block + j, (char*)out_buf + j * BLOCK_SIZE))
                    return -1;
            }
            return entries[i].size_bytes;
        }
    }
    return -1;
}


void kfs_test(HBA_PORT_T* port){
    // Format disk
    if(kfs_init(port, 1024)){   // 1024 blocks = 512 KiB
        printf("[KFS] KeblaFS initialized successfully\n");
    }else {
        printf("[KFS] Failed to initialize KeblaFS\n");
        return;
    }

    // Create file
    char msg[] = "Hello from KeblaFS!";
    kfs_create(port, "greet.txt", msg, sizeof(msg));

    // Read file
    char buf[512];
    int size = kfs_read(port, "greet.txt", buf);
    if (size > 0) {
        buf[size] = '\0';
        printf("[KFS] Read: %s\n", buf);
    }
    else {
        printf("[KFS] Failed to read file\n");
    }
}





