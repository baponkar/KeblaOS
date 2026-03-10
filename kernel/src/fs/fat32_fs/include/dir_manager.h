#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef struct {
    uint32_t first_cluster;     // First Cluster Number 
    uint32_t current_cluster;

    uint32_t pos;

    uint32_t parent_cluster;    // Parent Cluster of this directory entry

    char name[256];             // Directory long name
} FAT32_DIR;

// Directory Entry Info Structure
typedef struct {
    char name[256];
    uint8_t attr;
    uint32_t size;
    uint32_t first_cluster;
} FAT32_DIRENT;



bool f_cwd(char *path);                                     // get Current Working Directory
bool f_opendir(FAT32_DIR *dp, char *path);                  // Open Directory
bool f_closedir(FAT32_DIR *dp);                             // Close Directory
bool f_readdir(FAT32_DIR *dp, FAT32_DIRENT *entry);         // Reading Directory
bool f_mkdir(const char *path);                             // Make a new directory
bool f_rename(const char *oldpath, const char *newpath);    // rename file, rename directory, move file, move directory
bool f_chdir(const char *path);                             // Change Directory
char* f_getcwd(char *buff, uint32_t size);                  // Get Current Working Directory

bool f_findfirst(FAT32_DIR *dp, FAT32_DIRENT *entry, const char *path, const char *pattern);
bool f_findnext(FAT32_DIR *dp, FAT32_DIRENT *entry,  const char *pattern);










