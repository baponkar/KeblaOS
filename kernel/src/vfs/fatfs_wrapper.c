

#include "../memory/kheap.h"
#include "../fs/FatFs-R0.15b/source/ff.h"

#include "../lib/stdio.h"
#include "../lib/string.h"

#include "../lib/time.h"

#include "fatfs_wrapper.h"


extern FATFS *fatfs; // Global filesystem mount


int fatfs_mount(const char *path) {

    if(!fatfs){
        fatfs = (FATFS *)kheap_alloc(sizeof(FATFS), ALLOCATE_DATA);
        memset((void *)fatfs, 0, sizeof(FATFS));
    }
    

    FRESULT res = f_mount(fatfs, (const TCHAR*)path, 1);
    if (res != FR_OK) {
        printf("[FATFS] Failed to mount filesystem at %s, error: %d\n", path, res);
        return -1;
    }
    printf("[FATFS] Successfully mounted filesystem at %s\n", path);
    return 0;
}

int fatfs_unmount(const char *path) {
    FRESULT res = f_mount(NULL, path, 0);
    if (res != FR_OK) {
        printf("[FATFS] Failed to unmount filesystem at %s, error: %d\n", path, res);
        return -1;
    }
    printf("[FATFS] Successfully unmounted filesystem at %s\n", path);
    return 0;
}


int fatfs_open(vfs_node_t *node, int flags) {
    // ensure node and node->fs_data are valid
    if (!node) return -1;
    FIL *file = (FIL *)node->fs_data;
    if (!file) {
        file = (FIL *)kheap_alloc(sizeof(FIL), ALLOCATE_DATA);
        if (!file) return -1;
        node->fs_data = file;
    }
    memset(file, 0, sizeof(FIL)); 

    FRESULT res = f_open(file, node->name, flags);
    if (res != FR_OK) {
        printf("[FATFS] Failed to open file %s, error: %d\n", node->name, res);
        return -1;
    }

    node->is_dir = 0;
    node->is_open = 1;
    node->fs_data = (void *)file;

    return 0;
}

int fatfs_close(vfs_node_t *node) {
    FIL *file = (FIL *)node->fs_data;
    FRESULT res = f_close(file);
    node->is_open = 0;
    if (res != FR_OK) {
        printf("[FATFS] Failed to close file %s, error: %d\n", node->name, res);
        return -1;
    }
    return 0;
}

int fatfs_lseek(vfs_node_t *node, uint64_t offset){
    if(!node) return -1;
    return f_lseek((FIL *)node->fs_data, offset);
}

int fatfs_read(vfs_node_t *node, uint64_t offset, void *buf, uint64_t size) {

    FIL *file = (FIL *)node->fs_data;
    if(!file) {
        printf("node->fs_data is NULL\n");
        return -1;
    }
    
    if (f_lseek(file, offset) != FR_OK) {
        printf("[FATFS] Failed to seek in file %s\n", node->name);
        return -1;
    }

    UINT br;
    if (f_read(file, buf, size, &br) != FR_OK) {
        printf("[FATFS] Failed to read from file %s\n", node->name);
        return -1;
    }

    return br;      // Return number of bytes read
}

int fatfs_write(vfs_node_t *node, uint64_t offset, const void *buf, uint64_t size) {
    FIL *file = (FIL *)node->fs_data;
    UINT bw;
    FRESULT res = f_lseek(file, offset);
    if (res != FR_OK) {
        printf("[FATFS] Failed to seek in file %s, error: %d\n", node->name, res);
        return -1;
    }
    res = f_write(file, buf, size, &bw);
    if (res != FR_OK) {
        printf("[FATFS] Failed to write to file %s, error: %d\n", node->name, res);
        return -1;
    }
    return bw; // Return number of bytes written
}



int fatfs_opendir(vfs_node_t *node, int flags){
    if (!node) return -1;

    DIR *dir = (DIR *)node->fs_data;
    if(!dir){
        dir = (DIR *)kheap_alloc(sizeof(DIR), ALLOCATE_DATA);
        if(!dir) return -1;
        node->fs_data = (void *)dir;
    }
    memset(dir, 0, sizeof(DIR)); 

    FRESULT res = f_opendir(dir, (const TCHAR*) node->name);
    if (res != FR_OK) {
        printf("[FATFS] Failed to open directory %s, error: %d\n", node->name, res);
        return -1;
    }

    node->fs_data = (void *)dir;

    return 0;
}

int fatfs_readdir(vfs_node_t *dir_node, vfs_node_t **children, uint64_t *child_count) {

    if(!dir_node || !children || !child_count) return -1;

    DIR *dir = (DIR *) dir_node->fs_data;

    FILINFO fno;
    *child_count = 0;

    while (true) {
        memset(&fno, 0, sizeof(FILINFO));
        FRESULT res = f_readdir(dir, &fno);

        if (res != FR_OK || fno.fname[0] == 0) break; // End of directory or error

        // Allocate memory for child node
        vfs_node_t *child = (vfs_node_t *)kheap_alloc(sizeof(vfs_node_t),ALLOCATE_DATA);
        if (!child) {
            printf("[FATFS] Memory allocation failed for child node\n");
            return -1;
        }

        strncpy(child->name, fno.fname, sizeof(child->name) - 1);
        child->name[sizeof(child->name) - 1] = '\0'; // Ensure null termination
        child->is_dir = (fno.fattrib & AM_DIR) ? 1 : 0;
        child->size = fno.fsize;
        child->ctime = fno.ftime;   // Assuming ftime is in a compatible format
        child->fs_data = NULL;      // Set to NULL or appropriate filesystem data

        children[*child_count] = child;
        (*child_count)++;
    }

    f_closedir(dir);

    return 0;                       // Success
}


int fatfs_mkdir(vfs_node_t *parent, const char *name) {
    
    char full_path[512];
    sprintf(full_path, "%s/%s", parent->name, name);

    FRESULT res = f_mkdir(full_path);
    if (res != FR_OK) {
        printf("[FATFS] Failed to create directory %s, error: %d\n", full_path, res);
        return -1;
    }
    printf("[FATFS] Successfully created directory %s\n", full_path);
    return 0;
}










int fatfs_listdir(const char *path) {

    DIR dir;
    FILINFO fno;

    FRESULT res = f_opendir(&dir, path);
    if (res != FR_OK) {
        printf("[FATFS] Failed to open dir: %s, error: %d\n", path, res);
        return -1;
    }

    // printf("[FATFS] Listing directory: %s: ", path);

    while (1) {
        memset(&fno, 0, sizeof(FILINFO)); 
        res = f_readdir(&dir, &fno);

        if (res != FR_OK) {
            printf("[FATFS] Read error: %d\n", res);
            return -1;
            break;
        }

        if (fno.fname[0] == 0) break;

        // Dump first few raw bytes
        printf((fno.fattrib & AM_DIR) ? "%s " : "%s(%d bytes) " ,fno.fname, (uint64_t)fno.fsize);
    }

    printf("\n");

    f_closedir(&dir);

    return 0;
}


int fatfs_unlink(const char *file_path) {

    FRESULT res = f_unlink(file_path);
    if (res != FR_OK) {
        printf("[FATFS] Failed to delete file %s, error: %d\n", file_path, res);
        return -1;
    }
    printf("[FATFS] Successfully deleted file %s\n", file_path);
    return 0;
}


int fatfs_stat(vfs_node_t *node) {
    FILINFO fno;
    FRESULT res = f_stat(node->name, &fno);
    if (res != FR_OK) {
        printf("[FATFS] Failed to stat file %s, error: %d\n", node->name, res);
        return -1;
    }
    
    node->size = fno.fsize;
    node->ctime = fno.ftime; // Assuming ftime is in a compatible format
    node->is_dir = (fno.fattrib & AM_DIR) ? 1 : 0;
    
    return 0; // Success
}


int fatfs_rename(vfs_node_t *node, const char *new_name) {
    char old_path[512];
    sprintf(old_path, "%s/%s", node->parent->name, node->name);
    
    char new_path[512];
    sprintf(new_path, "%s/%s", node->parent->name, new_name);
    
    FRESULT res = f_rename(old_path, new_path);
    if (res != FR_OK) {
        printf("[FATFS] Failed to rename %s to %s, error: %d\n", old_path, new_path, res);
        return -1;
    }
    
    strncpy(node->name, new_name, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0'; // Ensure null termination
    printf("[FATFS] Successfully renamed %s to %s\n", old_path, new_path);
    return 0;
}


