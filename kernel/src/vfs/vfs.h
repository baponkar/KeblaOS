#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    VFS_UNKNOWN = 0,
    VFS_FAT12 = 1,
    VFS_FAT16 = 2,
    VFS_FAT32 = 3,
    VFS_EXFAT = 4,
    VFS_EXT2 = 5,
    VFS_EXT3 = 6,
    VFS_EXT4 = 7,
    VFS_NTFS = 8,
    VFS_UDF = 9,
    VFS_NFS = 10,
    VFS_SMB = 11,
    VFS_SSHFS = 12,
    VFS_WEBDEV = 13,
    VFS_APFS = 14,
    VFS_ISO9660 = 15,
    VFS_TEMPFS = 16,
    VFS_RAW = 17
}FS_TYPE;


typedef struct {
    FS_TYPE type;
    const char* name;
    uint64_t start_lba;
    uint32_t sector_size;
    bool valid;
} FS_INFO;


FS_TYPE detect_filesystem(int disk_no);

int vfs_init(int disk_no);

int vfs_disk_status(int disk_no);
int vfs_mount(int disk_no);
int vfs_mkfs(int disk_no, int fs_type);

#if FF_MULTI_PARTITION
int vfs_fdisk(int disk_no, void *ptbl, void* work)
#endif

int vfs_setcp(int disk_no, int cp);
int vfs_putc(int disk_no, void *fp, char c);
int vfs_puts(int disk_no, char *str, void *cp);
int vfs_printf(int disk_no, void *fp, char *str);
char *vfs_gets(int disk_no, char *buff, int len, void *fp);

void *vfs_open(int disk_no, char *path, int mode);
int vfs_close(int disk_no, void *fp);
int vfs_read(int disk_no, void *fp, char *buff, int size);
int vfs_write(int disk_no, void *fp, char *buff, int filesize);
int vfs_lseek(int disk_no, void *fp, int offset);
int vfs_truncate(int disk_no, void *fp);
int vfs_sync(int disk_no, void * fp);
void *vfs_opendir(int disk_no, char *path);
int vfs_closedir(int disk_no, void *dp);
int vfs_readdir(int disk_no, void *dp, void *fno);
int vfs_findfirst(int disk_no, void *dp, void *fno, char *path, char *pattern);
int vfs_findnext(int disk_no, void *dp, void *fno);
int vfs_mkdir(int disk_no, char *path);
int vfs_unlink(int disk_no, char *path);
int vfs_rename(int disk_no, char *old_path, char *new_path);
int vfs_stat(int disk_no, char *path, void *fno);
int vfs_chmod(int disk_no, char *path, int attr, int mask);
int vfs_utime(int disk_no, char *path, void *fno);
int vfs_chdir(int disk_no, char *path);
int vfs_chdrive(int disk_no, char *path);
int vfs_getcwd(int disk_no, char *buff, int len);
int vfs_getfree(int disk_no, char *path);
int vfs_getlabel(int disk_no, char *path, char* label, void *vsn);
int vfs_setlabel(int disk_no, char *label);
int vfs_forward(int disk_no);
int vfs_expand(int disk_no);

const char* vfs_error_string(int result);

void vfs_test(int disk_no);




