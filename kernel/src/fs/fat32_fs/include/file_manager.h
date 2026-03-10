#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stddef.h>



#define FA_READ     0x01
#define FA_WRITE    0x02
#define FA_CREATE   0x04
#define FA_CREATE_ALWAYS 0x08

#define FR_OK            0
#define FR_DISK_ERR     -1
#define FR_INT_ERR      -2
#define FR_NO_FILE      -3
#define FR_DENIED       -4
#define FR_NO_PATH      -5
#define FR_INVALID_OBJ  -6

typedef struct {
    uint32_t first_cluster;     // First Cluster number
    uint32_t current_cluster;   // cluster currently reading/writing
    uint32_t size;              // file size
    uint32_t pos;               // pointer position

    uint32_t parent_cluster;    // Parent Directory cluster

    uint32_t dir_entry_sector;  // sector containing the entry
    uint32_t dir_entry_offset;  // offset inside sector

    char name[256];             // Long Name
    uint8_t mode;               // read/write flags

    int error;                  // last error code
} FAT32_FILE;                   // 

typedef struct {
    char name[256];
    uint8_t attr;
    uint32_t size;
    uint32_t first_cluster;
} FAT32_STAT;



bool f_open( FAT32_FILE* fp, const char* path, int mode);
bool f_close(FAT32_FILE* fp);
bool f_read(FAT32_FILE* fp, void *buff, uint32_t btr, uint32_t *br);
bool f_write(FAT32_FILE* fp, const void* buff, uint32_t btw, uint32_t* bw);
bool f_lseek(FAT32_FILE* fp, uint32_t ofs);
bool f_truncate(FAT32_FILE* fp);
bool f_sync(FAT32_FILE *fp);
bool f_forward( FAT32_FILE *fp, uint32_t (*func)(const uint8_t *data, uint32_t len),  uint32_t btf,  uint32_t *bf);
bool f_expand(FAT32_FILE *fp, uint32_t size);
char* f_gets(char *buff, int len, FAT32_FILE *fp);
int f_putc(char c, FAT32_FILE *fp);
int f_puts(const char *str, FAT32_FILE *fp);
int f_printf(FAT32_FILE *fp, const char *fmt, ...);
uint32_t f_tell(FAT32_FILE *fp);
bool f_eof(FAT32_FILE *fp);
uint32_t f_size(FAT32_FILE *fp);
bool f_stat(const char *path, FAT32_STAT *stat);
bool f_unlink(const char *path);
int f_error(FAT32_FILE *fp);


