


#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../memory/kheap.h"

#include "../../../ext_lib/FatFs-R0.15b/source/ff.h"
#include "../../../ext_lib/FatFs-R0.15b/source/diskio.h"

#include "vfs.h"

#include "tree.h"



// Recursive helper
static void vfs_print_tree_recursive(const char *path, int depth) {
    vfs_node_t *dir_node = vfs_opendir((char *)path, FA_READ);
    if (!dir_node) {
        printf("[VFS] Failed to open directory: %s!\n", path);
        return;
    }

    vfs_node_t **children = NULL;
    uint64_t child_count = 0;

    if (vfs_read_dir(dir_node, &children, &child_count) != 0) {
        printf("[VFS] Failed to read directory: %s!\n", path);
        vfs_close(dir_node);
        return;
    }

    for (uint64_t i = 0; i < child_count; i++) {
        vfs_node_t *child = children[i];

        // Indentation
        for (int j = 0; j < depth; j++) {
            printf("  ");
        }

        if (child->is_dir) {
            printf("[DIR] %s\n", child->name);

            // Build next path correctly
            char next_path[256];
            if (strcmp(path, "/") == 0)
                snprintf(next_path, sizeof(next_path), "/%s", child->name);
            else
                snprintf(next_path, sizeof(next_path), "%s/%s", path, child->name);

            vfs_print_tree_recursive(next_path, depth + 1);
        } else {
            printf("%s (%llu bytes)\n", child->name,
                   (unsigned long long)child->size);
        }
    }

    kheap_free(children, sizeof(vfs_node_t *) * 128);
    vfs_close(dir_node);
}



// Public function: start from current working directory
void vfs_print_tree() {
    char cwd[256];
    if (vfs_getcwd(cwd, sizeof(cwd)) != 0) {
        printf("[VFS] Failed to get CWD\n");
        return;
    }

    printf("[VFS] Directory tree for: %s\n", cwd);
    vfs_print_tree_recursive(cwd, 0);
}

