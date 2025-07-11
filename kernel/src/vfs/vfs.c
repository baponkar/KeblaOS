
/*
Virtual File System

References:
    https://wiki.osdev.org/VFS

*/


#include "../FatFs-R0.15b/source/ff.h"
#include "../memory/kheap.h"

#include "../lib/stdio.h"
#include "../lib/string.h"

#include "vfs.h"


extern FATFS *fatfs; // Global filesystem mount

static vfs_node_t* fatfs_open(const char *path, uint64_t mode) {
    FIL *fp = (FIL *)kheap_alloc(sizeof(FIL), ALLOCATE_DATA);
    BYTE fat_mode = (BYTE) mode;

    FRESULT res = f_open(fp, path, fat_mode);
    if (res != FR_OK) {
        kheap_free(fp, sizeof(FIL));
        return NULL;
    }

    vfs_node_t *node = (vfs_node_t *)kheap_alloc(sizeof(vfs_node_t), ALLOCATE_DATA);
    strncpy(node->name, path, 255);
    node->is_dir = 0;
    node->fs_data = (void *)fp;
    node->read = &fatfs_read;
    node->write = &fatfs_write;
    node->close = &fatfs_close;
    node->lseek = &fatfs_lseek;

    return node;
}

static uint64_t fatfs_read(vfs_node_t *node, void *buf, uint64_t size) {
    UINT br = 0;
    FRESULT res = f_read((FIL *)node->fs_data, buf, size, &br);

    return (uint64_t)br;
}

static uint64_t fatfs_write(vfs_node_t *node, const void *buf, uint64_t size) {
    UINT bw = 0;
    f_write((FIL *)node->fs_data, buf, size, &bw);
    return (uint64_t)bw;
}

static uint64_t fatfs_close(vfs_node_t *node) {
    if (!node || !node->fs_data) return -1;
    FRESULT res = f_close((FIL *)node->fs_data);
    kheap_free(node->fs_data, sizeof(FIL));
    kheap_free(node, sizeof(FIL));

    return (uint64_t) res;
}

	
static uint64_t fatfs_lseek(vfs_node_t *node, uint64_t offset) {
    if ( !node || !node->fs_data) return -1;
    FRESULT res = f_lseek((FIL *)node->fs_data, (FSIZE_t) offset);

    return (res == FR_OK) ? 0 : -1;
}

static uint64_t fatfs_truncate(vfs_node_t *node) {
    if ( !node || !node->fs_data) return -1;
    FRESULT res = f_truncate((FIL *)node->fs_data);

    return (res == FR_OK) ? 0 : -1;
}

// -- new functins





void vfs_init() {
    uint64_t res = (uint64_t) f_mount(fatfs, "", 1);  // Assume fatfs is a global FATFS
    printf("Successfully implemented VFS: %x\n", res);
}

vfs_node_t* vfs_open(const char *path, uint64_t mode) {
    return fatfs_open(path, mode);
}

uint64_t vfs_read(vfs_node_t *node, void *buf, uint64_t size) {
    return node->read(node, buf, size);
}

uint64_t vfs_write(vfs_node_t *node, void *buf, uint64_t size) {
    return node->write(node, buf, size);
}

uint64_t vfs_close(vfs_node_t *node) {
    return node->close(node);
}

uint64_t vfs_lseek(vfs_node_t *node, uint64_t offset) {
    return node->lseek(node, offset);
}

uint64_t vfs_truncate(vfs_node_t *node, uint64_t offset) {
    node->lseek(node, offset);  // Set Pointer first
    return node->truncate(node);    // Truncate the file with current file pointer.
}


void test_vfs() {
    printf("Testing VFS\n");

    const char *file_name = "my_test_file.txt";
    char *msg = "Hello from message\n";
    char buf[128];

    vfs_init();

    uint64_t mode = (uint64_t) (FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
    vfs_node_t *node = vfs_open(file_name, mode);

    if(node == NULL){
        printf("node: %x, File open failed!\n", (uint64_t)node);
    }

    uint64_t res = vfs_write(node, (void *)msg, strlen(msg));
    if(res > 0) printf("Write Success\n");
    
    uint64_t res_1 = vfs_lseek(node, 0);
    if(res_1 == FR_OK) printf("Lseek Success\n");

    uint64_t res_2 = vfs_read(node, (void *)&buf, sizeof(buf) - 1);
    if(res_2 > 0) printf("Read Success\n");

    buf[res_2] = '\0';

    printf("Read from my_test_file.txt: %s\n", buf);

}



