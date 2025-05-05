#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern uint32_t current_dir_cluster; 

extern char current_path[256];

void init_vfs();
uint32_t vfs_get_current_cluster();
void vfs_set_current_cluster(uint32_t cluster);
void print_dir_tree(uint32_t cluster, int depth);
