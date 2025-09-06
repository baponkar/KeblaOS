
/*
Virtual File System

References:
    https://wiki.osdev.org/VFS

*/


#include "../../../ext_lib/FatFs-R0.15b/source/ff.h"
#include "../../../ext_lib/FatFs-R0.15b/source/diskio.h"

#include "../memory/kheap.h"

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/limit.h"
#include "../lib/errno.h"

#include "../fs/fatfs_wrapper.h"

#include "vfs.h"



vfs_node_t *vfs_root;


void vfs_init(char *fs_name) {

    // printf("[VFS] Starting VFS Initialization\n");

    vfs_root = (vfs_node_t *)kheap_alloc(sizeof(vfs_node_t), ALLOCATE_DATA);
    if (!vfs_root) {
        printf("[VFS] vfs_root Allocation failed\n");
        return;
    }
    memset((void *)vfs_root, 0, sizeof(vfs_node_t));

    // Use FAT Filesystem to initialize vfs
    if (fs_name && strncmp(fs_name, "fat", 3) == 0) {
        fatfs_init(0);

        vfs_fs_t *fatfs_ops = (vfs_fs_t *)kheap_alloc(sizeof(vfs_fs_t), ALLOCATE_DATA);
        if(!fatfs_ops){
            printf("[VFS] fatfs_ops Allocation failed!\n");
            return;
        }
        memset(fatfs_ops, 0, sizeof(vfs_fs_t));

        fatfs_ops->mount = fatfs_mount;
        fatfs_ops->unmount = fatfs_unmount;
        fatfs_ops->open = fatfs_open;
        fatfs_ops->close = fatfs_close;
        fatfs_ops->read = fatfs_read;
        fatfs_ops->write = fatfs_write;
        fatfs_ops->opendir = fatfs_opendir;
        fatfs_ops->readdir = fatfs_readdir;
        fatfs_ops->mkdir = fatfs_mkdir;
        fatfs_ops->unlink = fatfs_unlink;
        fatfs_ops->stat = fatfs_stat;
        fatfs_ops->rename = fatfs_rename;
        
        // more
        fatfs_ops->listdir = fatfs_listdir;
        fatfs_ops->lseek = fatfs_lseek;
        fatfs_ops->getcwd = fatfs_getcwd;
        fatfs_ops->chdir = fatfs_chdir;
        fatfs_ops->chdrive = fatfs_chdrive;
        fatfs_ops->mkfs = fatfs_mkfs;

        DIR *dp = (DIR *)kheap_alloc(sizeof(DIR), ALLOCATE_DATA);
        if(!dp){
            printf("dp Allocation failed!\n");
            return;
        }
        memset(dp, 0, sizeof(DIR));

        const char* name = "/";
        // safer copy: respect destination size
        strncpy(vfs_root->name, name, sizeof(vfs_root->name) - 1);
        vfs_root->name[sizeof(vfs_root->name) - 1] = '\0';

        vfs_root->is_open = 1;
        vfs_root->is_dir = 1;
        vfs_root->is_symlink = 0;
        vfs_root->size = sizeof(DIR);
        vfs_root->flags = FA_READ | FA_WRITE;
        vfs_root->ctime = 0;
        vfs_root->atime = 0;
        vfs_root->mtime = 0;
        vfs_root->parent = NULL;
        vfs_root->children = NULL;
        vfs_root->child_count = 0;
        vfs_root->fs_data = (void *)dp;

        vfs_root->fs = fatfs_ops;
    } else {
        printf("[VFS] NO filesystem mentioned in vfs_init. Using default FAT.\n");
    }

    printf("[VFS] Successfully Initialized VFS.\n");
}

int vfs_mkfs(int fat_type, const char *disk){

    if(fat_type < FM_FAT || disk == NULL){
        return -1;
    }

    if(vfs_root == NULL || vfs_root->fs == NULL || vfs_root->fs->mkfs == NULL){
        return -1;
    }

    if(!vfs_root){
        return -1;
    }

    printf("Inside of vfs_mkfs: fat_type %d, disk %s\n", fat_type, disk);
    
    // if(vfs_root->fs->mkfs(fat_type, disk) != 0){
    //     return -1;
    // }
    
    return 0;
}

vfs_node_t* vfs_get_root() {
    return vfs_root;
}


int vfs_mount(vfs_node_t *root_node, char *disk) {
    
    if(root_node == NULL || root_node->fs == NULL || root_node->fs->mount == NULL || disk == NULL){
        return -1;
    }

    if(root_node->fs->mount(disk) != 0){

        printf("[VFS] Failed mounted Disk with FAT Filesystem\n");
        return -1;
    }

    printf("[VFS] Successfully mounted Disk with FAT Filesystem.Disk: %s\n", disk);

    return 0;
}



int vfs_unmount(vfs_node_t *root_node, char *disk) {

    if(!root_node) return -1;

    if(root_node->fs->unmount(disk) != 0){
        printf("[VFS] Failed to Unmounting FAT Filesystem.\n");
        return -1;
    }

    printf("[VFS] Successfully Unmount the FAT filesystem.Disk: %s\n", disk);

    return 0;
}


vfs_node_t *vfs_create_node(char *path, int node_type){

    vfs_node_t *node = kheap_alloc(sizeof(vfs_node_t), ALLOCATE_DATA);
    if(!node) return NULL;
    memset(node, 0, sizeof(vfs_node_t));

    char *basename = strrchr(path, '/');
    if (basename)
        basename++; // skip '/'
    else
        basename = path;
    strncpy(node->name, basename, 255);
    node->name[255] = '\0'; // ensure null-termination

    node->is_dir = (node_type == 0) ? 1 : 0;

    node->fs = vfs_root->fs;

    return node;
}


// Opening a file
vfs_node_t *vfs_open(char *path, uint64_t flags){
    if(!path) return NULL;

    vfs_node_t *node = (vfs_node_t *) kheap_alloc(sizeof(vfs_node_t), ALLOCATE_DATA);
    if(!node) return NULL;
    memset(node, 0, sizeof(vfs_node_t));   // ensure it's zeroed

    // allocate file struct in node->fs_data
    node->fs_data = (void *) kheap_alloc(sizeof(FIL), ALLOCATE_DATA);
    if (!node->fs_data) {
        kheap_free(node, sizeof(vfs_node_t));
        return NULL;
    }
    memset(node->fs_data, 0, sizeof(FIL));

    // safe copy of path -> node->name
    strncpy(node->name, path, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0';
    node->fs = vfs_root->fs;

    if(fatfs_open(node, flags) != 0){
        // free any allocated resources
        if (node->fs_data) kheap_free(node->fs_data, sizeof(FIL));
        kheap_free(node, sizeof(vfs_node_t));
        return NULL;
    }

    return node;
}

int vfs_close(vfs_node_t *node) {
    if (!node) return -1;

    // Call the close function of the underlying FS if it exists
    if (node->fs && node->fs->close) {
        int res = node->fs->close(node);
        if (res != 0) return res;
    }

    // Free the vfs_node memory
    kheap_free(node, sizeof(vfs_node_t));
    return 0;
}

size_t vfs_read(vfs_node_t *node, uint64_t offset, void *buf, size_t size) {

    if (!node || !buf) {
        return -EINVAL;         // Invalid arguments
    }

    if (!node->fs || !node->fs->read) {
        return -ENOSYS;         // Filesystem doesn't support read
    }

    size_t res = node->fs->read(node, offset, buf, size);

    // printf("node: %x, offset: %x, buf: %x, size: %x\n", (uint64_t)node, offset, (uint64_t)buf, (uint64_t)size);

    return res;
}

int vfs_write(vfs_node_t *node, uint64_t offset, const void *buf, uint64_t size) {
    if (!node || !node->fs || !node->fs->write || !buf)
        return -1;

    return node->fs->write(node, offset, buf, size);
}

int vfs_lseek(vfs_node_t *node, uint64_t offset){
    if(!node) return -1;
    return fatfs_lseek(node, offset);
}

int vfs_unlink(const char *file_path){
    if(!file_path) return -1;
    return fatfs_unlink(file_path);
}

int vfs_mkdir(char *path){
    vfs_node_t *node = kheap_alloc(sizeof(vfs_node_t), ALLOCATE_DATA);
    if(!node) return -1;
    memset(node, 0, sizeof(vfs_node_t));

    // safe copy of path -> node->name
    strncpy(node->name, path, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0';
    node->is_dir = 1;
    node->is_open = 1;
    node->fs = vfs_root->fs;

    if(fatfs_mkdir(path) != 0){
        // free any allocated resources
        if (node->fs_data) kheap_free(node->fs_data, sizeof(FIL));
        kheap_free(node, sizeof(vfs_node_t));
        return -1;
    }

    return 0;
}



vfs_node_t *vfs_opendir(char *path, uint64_t flags){
    if(!path) return NULL;

    vfs_node_t *node = (vfs_node_t *) kheap_alloc(sizeof(vfs_node_t), ALLOCATE_DATA);
    if(!node) return NULL;
    memset(node, 0, sizeof(vfs_node_t));

    // safe copy of path -> node->name
    strncpy(node->name, path, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0';
    node->is_dir = 1;
    node->is_open = 1;
    node->fs = vfs_root->fs;

    if(fatfs_opendir(node, flags) != 0){
        // free any allocated resources
        if (node->fs_data) kheap_free(node->fs_data, sizeof(FIL));
        kheap_free(node, sizeof(vfs_node_t));
        return NULL;
    }

    return node;
}



int vfs_listdir(const char *path){
    if(!path) return -1;
    return fatfs_listdir(path);
}

int vfs_read_dir(vfs_node_t *dir_node, vfs_node_t ***children, uint64_t *child_count) {
    if (!dir_node || !children || !child_count) {
        return -1; // Invalid arguments
    }

    // Ensure it's a directory
    if (!dir_node->is_dir) {
        printf("[VFS] Node %s is not a directory\n", dir_node->name);
        return -1;
    }

    // Allocate an initial array for children pointers (caller can reallocate if needed)
    // Let's say we allow up to 128 entries for now (or you can make this dynamic)
    *children = (vfs_node_t **)kheap_alloc(sizeof(vfs_node_t *) * 128, ALLOCATE_DATA);
    if (!*children) {
        printf("[VFS] Memory allocation failed for children array\n");
        return -1;
    }

    *child_count = 0;

    // Call the FS-specific readdir
    if (dir_node->fs && dir_node->fs->readdir) {
        int res = dir_node->fs->readdir(dir_node, *children, child_count);
        if (res != 0) {
            kheap_free(*children, sizeof(vfs_node_t *) * 128); // Clean up on failure
            *children = NULL;
            *child_count = 0;
            return -1;
        }
    } else {
        printf("[VFS] Filesystem driver does not support readdir\n");
        kheap_free(*children, sizeof(vfs_node_t *) * 128);
        *children = NULL;
        return -1;
    }

    return 0; // Success
}

int vfs_getcwd(void *buf, size_t size){
    return fatfs_getcwd(buf, size);
}

int vfs_chdir(const char *path) {
    return fatfs_chdir(path);
}

int vfs_chdrive(const char *path){
    return fatfs_chdrive(path);
}


void vfs_write_log(char *log_buffer){
    const char log_path[] = "/log.txt";
    uint64_t flags = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;
    vfs_node_t *file_node = vfs_open((char *)log_path, flags);

    if(file_node){
        vfs_write(file_node, 0, (const void *)log_buffer, strlen(log_buffer));
    }

    vfs_close(file_node);
    printf("[VFS] Successfully Written log into /log.txt\n");
}



void vfs_test(){

    printf("[VFS] Testing Start....................\n");

    // Initialize FAT File System
    vfs_init("fat");

    // Format the Disk with FAT32 Filesystem
    vfs_mkfs(2, "0:");

    // Mounting the First Disk
    if(!vfs_mount(vfs_get_root(), "0:")){
        printf(" Successfully VFS Disk mounted.\n");
    }else{
        printf(" VFS Mounting Failed!\n");
    }
    
    // Get Current Working Directory
    char cwd[256];
    if(vfs_getcwd(cwd, sizeof(cwd)) == 0){
        printf(" Present Working Directory: %s\n", cwd);
    }else{
        printf(" Current Working Directory Failed!\n");
    }

    // Listing current working directory
    const char *root_path = "/";
    printf(" Listing %s directory: ", root_path);
    vfs_listdir(root_path);


    // ==========================================================================

    // Creating a testfile.txt
    const char* testfile_path = "/testfile.txt";
    uint64_t testfile_flags = FA_CREATE_NEW | FA_READ | FA_WRITE;
    vfs_node_t *testfile_node = vfs_open((char *)testfile_path, testfile_flags);
    if(!testfile_node){
        printf(" %s creation failed!\n", testfile_path);
    }

    // Writing the testfile.txt
    const char* testfile_data = "This is a test text file.";
    if(vfs_write(testfile_node, 0, (const void *)testfile_data, strlen(testfile_data)) <= 0){
        printf(" Writing testfile.txt is failed!\n");
    }

    // Reading the testfile.txt
    char testfile_buffer[512];
    if(vfs_read(testfile_node, 0, testfile_buffer, sizeof(testfile_buffer)) <= 0){
        printf(" testfile.txt read is failed!\n");
    }
    printf(" %s content: %s\n", testfile_path, testfile_buffer);

    // Close the testfile.txt
    if(vfs_close(testfile_node) != 0){
        printf(" Closing %s failed\n", testfile_path);
    }

    // =================================================================================

    // Creating a testdir directory
    const char *testdir_path = "/testdir_path";
    uint64_t testdir_flags = FA_CREATE_NEW | FA_READ | FA_WRITE | FA_OPEN_ALWAYS;
    if(vfs_mkdir(testdir_path) != 0){
        printf(" %s creation is failed!\n", testdir_path);
    }


    const char* nestedfile_path = "/testdir_path/nestedfile.txt";
    uint64_t nestedfile_flags = FA_CREATE_NEW | FA_READ | FA_WRITE;
    vfs_node_t *nestedfile_node = vfs_open((char *)nestedfile_path, nestedfile_flags);
    if(!nestedfile_node){
        printf(" %s creation failed!\n", nestedfile_path);
    }

    // Writing the nestedfile.txt
    const char* nestedfile_data = "This is a nested test text file.";
    if(vfs_write(nestedfile_node, 0, (const void *)nestedfile_data, strlen(nestedfile_data)) <= 0){
        printf(" Writing nestedfile.txt is failed!\n");
    }

    // Close the nestedfile.txt
    if(vfs_close(nestedfile_node) != 0){
        printf(" Closing %s failed\n", testfile_path);
    }

    // =================================================================================

    // Change Directory
    const char *nested_path = "/testdir_path";
    if(vfs_chdir(nested_path) != 0){
        printf(" Change Directory into %s failed!\n", nested_path);
    }

    
    // Get Current Working Directory
    memset(cwd,  0, sizeof(cwd));
    if(vfs_getcwd(cwd, sizeof(cwd)) == 0){
        printf(" Present Working Directory: %s\n", cwd);
    }else{
        printf(" Current Working Directory Failed!\n");
    }


    printf(" Listing %s directory: ", "/testdir_path");
    vfs_listdir("/testdir_path");

    printf("[VFS] Testing End......................\n");
}










