#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    VFS_UNKNOWN,
    VFS_FAT12,
    VFS_FAT16,
    VFS_FAT32,
    VFS_EXFAT,
    VFS_EXT2,
    VFS_EXT3,
    VFS_EXT4,
    VFS_NTFS,
    VFS_UDF,
    VFS_NFS,
    VFS_SMB,
    VFS_SSHFS,
    VFS_WEBDEV,
    VFS_APFS,
    VFS_ISO9660,
    VFS_TEMPFS,
    VFS_RAW
}VFS_TYPE;


typedef struct {
    VFS_TYPE type;
    const char* name;
    uint64_t start_lba;
    uint32_t sector_size;
    bool valid;
} FS_INFO;

typedef struct {
    VFS_TYPE type;
    void *context;
}VFS_FILE;


VFS_TYPE detect_filesystem(int disk_no);

int vfs_init(int disk_no);

int vfs_disk_status(int disk_no);
int vfs_mount(int disk_no);
int vfs_unmount(int disk_no);
int vfs_mkfs(int disk_no, VFS_TYPE fs_type);

#if FF_MULTI_PARTITION
int vfs_fdisk(int disk_no, void *ptbl, void* work);
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

#if FF_USE_FIND
int vfs_findfirst(int disk_no, void *dp, void *fno, char *path, char *pattern);
int vfs_findnext(int disk_no, void *dp, void *fno);
#endif 

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

int vfs_get_fsize(int disk_no, void *fp);


const char* vfs_error_string(int result);

void vfs_test(int disk_no);




