
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "FatFs-R.0.16/source/ff.h"

static const char* const error_strings[] = {
    "OK",                       /* FR_OK */
    "Disk error",               /* FR_DISK_ERR */
    "Internal error",           /* FR_INT_ERR */
    "Drive not ready",          /* FR_NOT_READY */
    "File not found",           /* FR_NO_FILE */
    "Path not found",           /* FR_NO_PATH */
    "Invalid name",             /* FR_INVALID_NAME */
    "Access denied",            /* FR_DENIED */
    "File already exists",      /* FR_EXIST */
    "Invalid object",           /* FR_INVALID_OBJECT */
    "Write protected",          /* FR_WRITE_PROTECTED */
    "Invalid drive",            /* FR_INVALID_DRIVE */
    "Not enabled",              /* FR_NOT_ENABLED */
    "No filesystem",            /* FR_NO_FILESYSTEM */
    "MKFS aborted",             /* FR_MKFS_ABORTED */
    "Timeout",                  /* FR_TIMEOUT */
    "File locked",              /* FR_LOCKED */
    "Not enough core",          /* FR_NOT_ENOUGH_CORE */
    "Too many open files",      /* FR_TOO_MANY_OPEN_FILES */
    "Invalid parameter"         /* FR_INVALID_PARAMETER */
};



const char* fatfs_error_string(FRESULT result);
int fatfs_init(int pdrv);
int fatfs_disk_status(int disk_no);

int fatfs_mkfs(int ld, int fs_type);
int fatfs_mount(int ld, int mount_opt);
int fatfs_unmount(int ld);


#if FF_MULTI_PARTITION
int fatfs_fdisk(int physical_disk_no, void *ptbl, void* work);
#endif

int fatfs_setcp(int cp);

#if FF_USE_STRFUNC
int fatfs_putc(void *fp, char c);
int fatfs_puts(char *str, void *cp);
char *fatfs_gets(char *buff, int len, void *fp);
#endif

#if FF_PRINT_LLI
int fatfs_printf(void *fp, char *str);
#endif

void *fatfs_open(char *path, int mode);
int fatfs_close(void *fp);
int fatfs_read(void *fp, char *buff, int size);
int fatfs_write(void *fp, char *buff, int filesize);
int fatfs_lseek(void *fp, int offset);
int fatfs_truncate(void *fp);
int fatfs_sync(void * fp);
void *fatfs_opendir(char *path);
int fatfs_closedir(void *dp);
int fatfs_readdir(void *dp, void *fno);

#if FF_USE_FIND
int fatfs_findfirst(void *dp, void *fno, char *path, char *pattern);
int fatfs_findnext(void *dp, void *fno);
#endif

int fatfs_mkdir(char *path);
int fatfs_unlink(char *path);
int fatfs_rename(char *old_path, char *new_path);
int fatfs_stat(char *path);
int fatfs_chmod(char *path, int attr, int mask);
int fatfs_utime(char *path, void *fno);
int fatfs_chdir(char *path);
#if F_MULTI_PARTITION
int fatfs_chdrive(char *path);
#endif
int fatfs_getcwd(char *buff, int len);
int fatfs_getfree(char *path);
int fatfs_getlabel(char *path, char* label, void *vsn);
int fatfs_setlabel(char *label);
int fatfs_forward();
int fatfs_expand();
int fatfs_get_fsize(void *fp);

int fatfs_listdir(char *path);



#if FF_MULTI_PARTITION
void fatfs_test_1(int disk_no);
#endif

void fatfs_check_disk_status(int physical_disk, int logical_drive);

void fatfs_test(int disk_no);



void my_fatfs_test();


































