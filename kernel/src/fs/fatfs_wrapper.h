
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "FatFs-R.0.16/source/ff.h"

DWORD get_fattime(void);


int fatfs_init(int disk_no);
int fatfs_disk_status(int disk_no);

int fatfs_mount(int disk_no);
int fatfs_unmount(int disk_no);
int fatfs_mkfs(int disk_no, int fs_type);

#if FF_MULTI_PARTITION
int fatfs_fdisk(int disk_no, void *ptbl, void* work);
#endif

int fatfs_setcp(int cp);

#if FF_PRINT_LLI
int fatfs_printf(void *fp, char *str);
#endif

#if FF_USE_STRFUNC
int fatfs_putc(void *fp, char c);
int fatfs_puts(char *str, void *cp);
char *fatfs_gets(char *buff, int len, void *fp);
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
int fatfs_stat(char *path, void *fno);
int fatfs_chmod(char *path, int attr, int mask);
int fatfs_utime(char *path, void *fno);
int fatfs_chdir(char *path);
int fatfs_chdrive(char *path);
int fatfs_getcwd(char *buff, int len);
int fatfs_getfree(char *path);
int fatfs_getlabel(char *path, char* label, void *vsn);
int fatfs_setlabel(char *label);
int fatfs_forward();
int fatfs_expand();

int fatfs_get_fsize(void *fp);

int fatfs_listdir(char *path);
const char* fatfs_error_string(FRESULT result);


void fatfs_test(int disk_no);

void fatfs_test_1(int disk_no);
void fatfs_comprehensive_test(int disk_no);




































