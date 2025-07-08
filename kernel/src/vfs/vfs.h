
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct vfs_node vfs_node_t;
typedef struct time time_t;

static uint64_t fatfs_read(vfs_node_t *node, void *buf, uint64_t size);
static uint64_t fatfs_write(vfs_node_t *node, const void *buf, uint64_t size);
static uint64_t fatfs_close(vfs_node_t *node);
static uint64_t fatfs_lseek(vfs_node_t *node, uint64_t offset);

struct vfs_node {
    char name[256];
    uint8_t is_dir;
    uint64_t size;
    void *fs_data;  // Points to FATFS or FIL depending on node type

    vfs_node_t *(*open)(const char *path, uint64_t mode);
    uint64_t (*read)(struct vfs_node *node, void *buf, uint64_t size);
    uint64_t (*write)(struct vfs_node *node, const void *buf, uint64_t size);
    uint64_t (*close)(struct vfs_node *node);
    uint64_t (*lseek)(struct vfs_node *node, uint64_t offset);
};

void vfs_init();
vfs_node_t* vfs_open(const char *path, uint64_t mode);
uint64_t vfs_read(vfs_node_t *node, void *buf, uint64_t size);
uint64_t vfs_write(vfs_node_t *node, void *buf, uint64_t size);
uint64_t vfs_close(vfs_node_t *node);
uint64_t vfs_lseek(vfs_node_t *node, uint64_t offset);

void test_vfs();


