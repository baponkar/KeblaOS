#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "diskio.h"
#include "partition_manager.h"
#include "fat32_mount.h"
#include "file_manager.h"
#include "dir_manager.h"




void fat32_set_disk(int no);


// Available functions in partition_manager.h
bool create_partition(uint8_t pdrv_no,                          // Creating a GPT Partition
    uint64_t start_lba, 
    uint64_t sectors, 
    const guid_t partition_guid, 
    const guid_t partition_type_guid, 
    char* name);
bool update_partition(int disk_no,
    size_t partition_index,                   // Update an existing GPT Partition 
    uint64_t new_start_lba, 
    uint64_t new_sectors, 
    const char* new_name,  
    uint64_t new_attributes);


    
// Available functions in fat32_mount.h
bool create_fat32_volume( uint32_t start_lba, uint32_t sectors); // defined in fat32_mount.c
bool fat32_mount( uint64_t partition_lba_start);                 // defined in fat32_mount.c


// Available in dir_manager.h
bool f_cwd(char *path);                                     // get Current Working Directory
bool f_opendir(FAT32_DIR *dp, char *path);                  // Open Directory
bool f_closedir(FAT32_DIR *dp);                             // Close Directory
bool f_readdir(FAT32_DIR *dp, FAT32_DIRENT *entry);         // Reading Directory
bool f_mkdir(const char *path);                             // Make a new directory
bool f_rename(const char *oldpath, const char *newpath);    // rename file, rename directory, move file, move directory
bool f_chdir(const char *path);                             // Change Directory
bool f_findfirst(FAT32_DIR *dp,                             // Pattern Search
    FAT32_DIRENT *entry, 
    const char *path, 
    const char *pattern);
bool f_findnext(FAT32_DIR *dp,                              // Pattern Search
    FAT32_DIRENT *entry,  
    const char *pattern);


// Available functions in file_manager.h
bool f_open( FAT32_FILE* fp, const char* path, int mode);   // Open a File
bool f_close(FAT32_FILE* fp);                               // Close opened file
bool f_read(FAT32_FILE* fp, void *buff, uint32_t btr, uint32_t *br);        // Read the file and content take in buffer
bool f_write(FAT32_FILE* fp, const void* buff, uint32_t btw, uint32_t* bw); // Write file from buffer position from the filepointer position
bool f_lseek(FAT32_FILE* fp, uint32_t ofs);                 // Change file pointer position
bool f_truncate(FAT32_FILE* fp);                            // Truncated the file w.r.t file pointer
bool f_sync(FAT32_FILE *fp);                                // Update FAT32 FS with inclusion the file
bool f_forward( FAT32_FILE *fp,                             // Instead of copying data into a buffer, it forwards file data directly to a user callback function.
    uint32_t (*func)(const uint8_t *data, 
    uint32_t len),  
    uint32_t btf,  
    uint32_t *bf);
bool f_expand(FAT32_FILE *fp, uint32_t size);       // is used to pre-allocate disk space for a file.
char* f_gets(char *buff, int len, FAT32_FILE *fp);  // f_gets() reads a text line from a file, similar to the standard C fgets().
int f_putc(char c, FAT32_FILE *fp);                 // f_putc() writes one character to a file, similar to the standard C fputc().
int f_puts(const char *str, FAT32_FILE *fp);        // f_puts() writes a null-terminated string to a file, similar to the standard C fputs().
int f_printf(FAT32_FILE *fp, const char *fmt, ...); // f_printf() works like printf(), but instead of printing to the console it writes formatted text to a file.
uint32_t f_tell(FAT32_FILE *fp);                    // f_tell() returns the current file position (offset) inside the file, similar to the standard C ftell().
bool f_eof(FAT32_FILE *fp);                         // f_eof() checks whether the file pointer has reached the end of the file.
uint32_t f_size(FAT32_FILE *fp);                    // f_size() returns the size of the opened file in bytes.
bool f_stat(const char *path, FAT32_STAT *stat);    // f_stat() retrieves information about a file or directory without opening it. It is similar to POSIX stat().
bool f_unlink(const char *path);                    // f_unlink() deletes a file (or empty directory) from the FAT32 filesystem.
int f_error(FAT32_FILE *fp);

// Defined in fat32.c
void fat32_fs_test(int disk_no, uint32_t start_lba, uint32_t sectors);




