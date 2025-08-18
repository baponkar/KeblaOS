#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef struct vfs_fs vfs_fs_t;
typedef struct vfs_node vfs_node_t;


struct vfs_fs {
    const char *name;

    int (*mount)(const char *path);
    int (*unmount)(const char *path);

    int (*open)(vfs_node_t *node, int flags);
    int (*close)(vfs_node_t *node);
    int (*read)(vfs_node_t *node, uint64_t offset, void *buf, uint64_t size);
    int (*write)(vfs_node_t *node, uint64_t offset, const void *buf, uint64_t size);

    int (*opendir)(vfs_node_t *node, int flags);
    int (*readdir)(vfs_node_t *dir_node, vfs_node_t **children, uint64_t *child_count);
    int (*mkdir)(vfs_node_t *parent, const char *name);

    int (*unlink)(const char *file_path);
    int (*stat)(vfs_node_t *node);
    int (*rename)(vfs_node_t *node, const char *new_name);
    
    // more functions added
    int (*listdir)(const char *path);
    int (*lseek)(vfs_node_t *node, uint64_t offset);
};



struct vfs_node {
    char name[256];             // File or directory name
    uint8_t is_open;
    uint8_t is_dir;             // 1 = directory, 0 = file
    uint8_t is_symlink;         // 1 = symlink (optional, useful for EXT2)
    uint64_t size;              // File size in bytes
    uint64_t mode;              // Permissions / access mode (e.g., 0644)
    uint64_t flags;             // Open flags (O_RDONLY, O_WRONLY, etc.)
    uint64_t ctime;             // Creation time
    uint64_t atime;             // Last access time
    uint64_t mtime;             // Last modification time

    struct vfs_node *parent;    // Pointer to parent node
    struct vfs_node **children; // Array of children if directory
    uint64_t child_count;       // Number of children

    void *fs_data;              // Filesystem-specific data (e.g., FIL* for FatFs, inode_t* for EXT2)
    vfs_fs_t *fs;               // Pointer to filesystem driver that manages this node
};


void vfs_init(char *fs_name);

vfs_node_t* vfs_get_root();

int vfs_mount(vfs_node_t *root_node, char *disk);
int vfs_unmount(vfs_node_t *root_node, char *disk);

vfs_node_t *create_node(char *path, int node_type);

vfs_node_t *vfs_open(char *path, uint64_t flags);
int vfs_close(vfs_node_t *node);
size_t vfs_read(vfs_node_t *node, uint64_t offset, void *buf, size_t size);
int vfs_write(vfs_node_t *node, uint64_t offset, const void *buf, uint64_t size);
int vfs_lseek(vfs_node_t *node, uint64_t offset);
int vfs_unlink(const char *file_path);

int vfs_mkdir(char *path);
vfs_node_t *vfs_opendir(char *path, uint64_t flags);
int vfs_listdir(const char *path);
int vfs_read_dir(vfs_node_t *dir_node, vfs_node_t ***children, uint64_t *child_count);

void vfs_write_log(char *log_buffer);

void vfs_test();




