
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "vfs.h"

void fatfs_init();

int fatfs_mount(const char *path);
int fatfs_unmount(const char *path);

int fatfs_open(vfs_node_t *node, int flags);
int fatfs_close(vfs_node_t *node);
int fatfs_lseek(vfs_node_t *node, uint64_t offset);
int fatfs_read(vfs_node_t *node, uint64_t offset, void *buf, uint64_t size);
int fatfs_write(vfs_node_t *node, uint64_t offset, const void *buf, uint64_t size);
int fatfs_create(vfs_node_t *parent, const char *name);
int fatfs_unlink(const char *file_path);

int fatfs_opendir(vfs_node_t *node, int flags);
int fatfs_readdir(vfs_node_t *dir_node, vfs_node_t **children, uint64_t *child_count);
int fatfs_mkdir(vfs_node_t *parent, const char *name);
int fatfs_listdir(const char *path);
 
int fatfs_pwd(void *buf);

int fatfs_rename(vfs_node_t *node, const char *new_name);
int fatfs_stat(vfs_node_t *node);

int fatfs_getcwd(void *buf, size_t size);
int fatfs_chdir(const char *path);
int fatfs_chdrive(const char *path);


