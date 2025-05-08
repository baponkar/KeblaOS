/*

*/

#include "../lib/stdio.h"
#include "../lib/string.h"

#include "../sys/ahci/ahci.h"
#include "fat32.h"

#include "vfs.h"

#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20

extern fat32_info_t fat32_info;

uint32_t current_dir_cluster; // Root cluster defined by FAT32

char current_path[256] = "/";


void init_vfs() {
    current_dir_cluster = fat32_info.root_dir_first_cluster; // Initialize to root directory
    strcpy(current_path, "/"); // Initialize current path to root
}

uint32_t vfs_get_current_cluster() {
    return current_dir_cluster;
}

void vfs_set_current_cluster(uint32_t cluster) {
    current_dir_cluster = cluster;
}


void print_dir_tree(uint32_t cluster, int depth) {
    DIR_ENTRY entries[16]; // Read cluster entries
    fat32_read_cluster(cluster, (uint8_t*)entries, sizeof(entries));

    for(int i = 0; i < 16; i++) {
        if(entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) continue;
        if(entries[i].attr & ATTR_DIRECTORY) {
            for(int j = 0; j < depth; j++) printf("  ");
            printf("[DIR] %s\n", entries[i].name);
            print_dir_tree(entries[i].fstClusLO, depth + 1);
        } else {
            for(int j = 0; j < depth; j++) printf("  ");
            printf("- %s\n", entries[i].name);
        }
    }
}
