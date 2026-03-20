
/*
Virtual File System

References:
    https://wiki.osdev.org/VFS
*/

#include "../driver/disk/disk.h"                

#include "../fs/iso9660/iso9660.h"
#include "../fs/fat32_fs/include/fat32.h"

#include "../memory/kheap.h"

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/limit.h"
#include "../lib/errno.h"

#include "vfs.h"


#define SECTOR_SIZE 512
#define MAX_PATH 256


bool is_gpt_disk(int disk_no){
    uint8_t sector[SECTOR_SIZE];

    if(!kebla_disk_read(disk_no, 0, 1, sector)){
        return false;
    }
}


VFS_TYPE detect_filesystem(int disk_no) {
    uint8_t sector[SECTOR_SIZE];

    // --- Step 1: read LBA 0 (MBR or Boot Sector) ---
    if (!kebla_disk_read(disk_no, 0, 1, sector))
        return VFS_UNKNOWN;

    // --- Step 2: check MBR signature (0x55AA) ---
    bool has_mbr = (sector[510] == 0x55 && sector[511] == 0xAA);
    if (has_mbr && sector[0x1BE + 4] != 0x00) {
        // Partition type field
        uint8_t ptype = sector[0x1BE + 4];
        uint32_t start_lba = *(uint32_t*)&sector[0x1BE + 8];

        // Read first sector of partition
        uint8_t pboot[SECTOR_SIZE];
        if (kebla_disk_read(disk_no, start_lba, 1, pboot)) {
            if (memcmp(&pboot[0x52], "FAT32", 5) == 0 ||
                memcmp(&pboot[0x36], "FAT32", 5) == 0)
                return VFS_FAT32;
            if (memcmp(&pboot[0x36], "FAT16", 5) == 0)
                return VFS_FAT16;
            if (memcmp(&pboot[0x36], "FAT12", 5) == 0)
                return VFS_FAT12;
        }
    }

    // --- Step 3: Check raw boot sector (superfloppy case) ---
    if (memcmp(&sector[0x52], "FAT32", 5) == 0 ||
        memcmp(&sector[0x36], "FAT32", 5) == 0)
        return VFS_FAT32;

    if (memcmp(&sector[0x36], "FAT16", 5) == 0)
        return VFS_FAT16;

    if (memcmp(&sector[0x36], "FAT12", 5) == 0)
        return VFS_FAT12;

    // --- exFAT ---
    if (memcmp(&sector[0x03], "EXFAT   ", 8) == 0)
        return VFS_EXFAT;

    // --- NTFS ---
    if (memcmp(&sector[0x03], "NTFS    ", 8) == 0)
        return VFS_NTFS;

    // --- ext2/3/4 (superblock at 1024 bytes) ---
    uint8_t extbuf[1024 + SECTOR_SIZE];
    if (kebla_disk_read(disk_no, 2, 2, extbuf)) {
        uint16_t magic = *(uint16_t*)&extbuf[1024 + 0x38];
        if (magic == 0xEF53) return VFS_EXT2;
    }

    // --- ISO9660 ---
    uint8_t sector16[SECTOR_SIZE];
    if (kebla_disk_read(disk_no, 16, 1, sector16)) {
        if (memcmp(&sector16[0x01], "CD001", 5) == 0)
            return VFS_ISO9660;
    }

    return VFS_RAW;
}


int vfs_init(int disk_no) {
    
    if (disk_no >= disk_count || !disks){
        printf("VFS Error: disk_no: %d, !disks: %d\n", disk_no, (uint64_t)!disks);
        return -1;
    } 

    Disk disk = disks[disk_no];

    if (disk.type == DISK_TYPE_SATAPI) {
        return iso9660_init(disk_no) == 0 ? 0 : -1;
    } else if (disk.type == DISK_TYPE_AHCI_SATA) {
        return 0;
    }
    printf("VFS: Unsupported disk type %d for init on disk %d\n", disk.type, disk_no);
    return -1;
}



int vfs_disk_status(int disk_no){
    if(!kebla_disk_status(disk_no)){
        printf(" VFS: Disk %d status check failed!\n", disk_no);
        return -1;
    }
    return 0;
}



int vfs_mount(int disk_no, uint32_t lba, VFS_TYPE type){

    if(disk_no >= disk_count || !disks) {
        return -1;
    }

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_mount(disk_no);
    }else if(disk.type == DISK_TYPE_AHCI_SATA || type == VFS_FAT32){
        set_disk_no(disk_no);
        return fat32_mount(lba) ? 0 : -1;
    }else{
        printf("VFS: Unsupported disk type %d for mount on disk %d\n", disk.type, disk_no);
        return -1;
    }

    return 0;
}



int vfs_unmount(int disk_no, int logical_drive){
    if(disk_no >= disk_count || !disks) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_unmount(disk_no);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return 0;
    }

    return -1;
}


int vfs_mkfs(int pd, uint32_t start_lba, uint32_t sectors, VFS_TYPE fs_type){

    if (pd >= disk_count || sectors <= 0 || !disks ) return -1;

    Disk disk = disks[pd];

    if(disk.type == DISK_TYPE_SATAPI){
        printf("FATFS: MKFS function is not effective in SATAPI Disk\n");
        return -1;
    }else{
        switch(fs_type){
            case VFS_UNKNOWN:
                printf("VFS: Cannot create filesystem of type UNKNOWN on disk %d\n", pd);
                return -1;
            case VFS_FAT12:
                return 0;
            case VFS_FAT16:
                return 0;
            case VFS_FAT32:
                set_disk_no(pd);
                return create_fat32_volume(start_lba, sectors) ? 0 : -1;
            case VFS_EXFAT:
                return 0;
            default:
                printf("VFS: Unsupported filesystem type %d for mkfs on disk %d\n", fs_type, pd);
                return -1;
        }
    }

    return 0;
}




// Set code page for the given disk
int vfs_setcp(int disk_no, int cp){
    if(disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];

    switch(disk.type){
        case DISK_TYPE_AHCI_SATA:
            return 0;
            break;
        default:
            printf("VFS: Unsupported disk type %d for setcp on disk %d\n", disk.type, disk_no);
            return -1;
    }
}





void *vfs_open(int pd_no, const char *path, int mode){

    if (pd_no >= disk_count || !disks || !path || mode < VFS_READ){
        return NULL;
    }
        
    Disk disk = disks[pd_no];

    if (disk.type == DISK_TYPE_SATAPI) {
        return iso9660_open(pd_no, path);
    }
    else if (disk.type == DISK_TYPE_AHCI_SATA) {

        FAT32_FILE *fp = malloc(sizeof(FAT32_FILE));
        if(!fp){
            printf("Memory allocation failed for fp!\n");
            return NULL;
        }
        memset(fp, 0, sizeof(FAT32_FILE));

        if (!f_open(fp, path, mode)) {
            free(fp);
            return NULL;
        }

        return (void *) fp;

    } else {
        printf("VFS: Unsupported disk type %d for open on disk %d\n", disk.type, pd_no);
        return NULL;
    }
}


int vfs_close(int disk_no, void *fp){

    if(!fp) return -1;

    Disk disk = disks[disk_no];

    switch(disk.type){
        case DISK_TYPE_SATAPI:
            return iso9660_close(fp);
            break;
        case DISK_TYPE_AHCI_SATA:
            FAT32_FILE *file = (FAT32_FILE *)fp;
            bool res = f_close(file);
            free(fp);
            return  res ? 0 : -1;
            break;
        default:
            printf("VFS: Unsupported disk type %d for close on disk %d\n", disk.type, disk_no);
            return -1;
    }
}


int vfs_read(int disk_no, void *fp, char *buff, int size){
    Disk disk = disks[disk_no];
    switch(disk.type){
        case DISK_TYPE_SATAPI:
            return iso9660_read(fp, buff, size);
            break;
        case DISK_TYPE_AHCI_SATA:
            uint32_t br;
            return f_read(fp, buff, size, &br) ? 0 : -1;
            break;
        default:
            printf("VFS: Unsupported disk type %d for read on disk %d\n", disk.type, disk_no);
            return -1;
    }
}

int vfs_write(int disk_no, void *fp, char *buff, int filesize){
    if(!fp || !buff || filesize <= 0) return -1;
    Disk disk = disks[disk_no];
    switch(disk.type){
        case DISK_TYPE_SATAPI:
            // ISO9660 is read-only
            printf("VFS: Write operation not supported on ISO9660 (disk %d)\n", disk_no);
            return -1;
            break;
        case DISK_TYPE_AHCI_SATA:
            uint32_t bw;
            return f_write(fp, buff, filesize, &bw) ? 0 : -1;
            break;
        default:
            printf("VFS: Unsupported disk type %d for write on disk %d\n", disk.type, disk_no);
            return -1;
    }
}



int vfs_lseek(int disk_no, void *fp, int offset){
    Disk disk = disks[disk_no];
    switch(disk.type){
        case DISK_TYPE_SATAPI:
            // ISO9660 file pointer management can be added here
            printf("VFS: Lseek operation not implemented for ISO9660 (disk %d)\n", disk_no);
            return -1;
            break;
        case DISK_TYPE_AHCI_SATA:
            return f_lseek(fp, offset) ? 0 : -1;
            break;
        default:
            printf("VFS: Unsupported disk type %d for lseek on disk %d\n", disk.type, disk_no);
            return -1;
    }
}


int vfs_truncate(int disk_no, void *fp){
    Disk disk = disks[disk_no];
    switch(disk.type){
        case DISK_TYPE_SATAPI:
            // ISO9660 is read-only
            printf("VFS: Truncate operation not supported on ISO9660 (disk %d)\n", disk_no);
            return -1;
            break;
        case DISK_TYPE_AHCI_SATA:
            return f_truncate(fp) ? 0 : -1;
            break;
        default:
            printf("VFS: Unsupported disk type %d for truncate on disk %d\n", disk.type, disk_no);
            return -1;
    }
}


int vfs_sync(int disk_no, void * fp){
    Disk disk = disks[disk_no];
    switch(disk.type){
        case DISK_TYPE_SATAPI:
            // ISO9660 is read-only
            printf("VFS: Sync operation not supported on ISO9660 (disk %d)\n", disk_no);
            return -1;
            break;
        case DISK_TYPE_AHCI_SATA:
            return f_sync(fp) ? 0 : -1;
            break;
        default:
            printf("VFS: Unsupported disk type %d for sync on disk %d\n", disk.type, disk_no);
            return -1;
    }
}

void *vfs_opendir(int disk_no, char *path){
    if(disk_no >= disk_count) return NULL;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_opendir(disk_no, path);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        FAT32_DIR dir;
        return f_opendir(&dir, path) ? &dir : NULL;
    }

    return NULL;
}

int vfs_closedir(int disk_no, void *dp){
    if(!dp) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_closedir(dp);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return f_closedir(dp) ? 0 : -1;
    }

    return -1;
}

int vfs_readdir(int disk_no, void *dp, void *fno){
    if(!dp | !fno) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_readdir(dp, fno);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return f_readdir(dp, fno) ? 0 : -1;
    }

    return -1;
}

int vfs_mkdir(int disk_no, char *path){
    if(!path || !disks) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Mkdir operation not supported on ISO9660 (disk %d)\n", disk_no);
        return -1;
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return 0;
    }else{
        printf("VFS: Unsupported disk type %d for mkdir on disk %d\n", disk.type, disk_no);
        return -1;
    }

    return -1;
}



int vfs_unlink(int disk_no, char *path){
    if(!path) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Unlink operation not supported on ISO9660 (disk %d)\n", disk_no);
        return -1;
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return f_unlink(path) ? 0 : -1;
    }

    printf("VFS: Unsupported disk type %d for unlink on disk %d\n", disk.type, disk_no);
    return -1;
}

int vfs_rename(int disk_no, char *old_path, char *new_path){
    if(!old_path | !new_path) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Rename operation not supported on ISO9660 (disk %d)\n", disk_no);
        return -1;
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return f_rename(old_path, new_path) ? 0 : -1;
    }

    return -1;
}

int vfs_stat(int disk_no, char *path, void *fno){
    if(!path) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_stat(disk_no, path, fno);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return f_stat(path, fno) ? 0 : -1;
    }

    return -1;
}

int vfs_chmod(int disk_no, char *path, int attr, int mask){
    if(!path) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Chmod operation not supported on ISO9660 (disk %d)\n", disk_no);
        return -1;
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return 0;
    }

    return -1;
}

int vfs_utime(int disk_no, char *path, void *fno){
    if(!path | !fno) return -1;
    Disk disk = disks[disk_no];
    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Utime operation not supported on ISO9660 (disk %d)\n", disk_no);
        return -1;
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return 0;
    }else{
        return -1;
    }
}

int vfs_chdir(int disk_no, char *path){
    Disk disk = disks[disk_no];
     if(disk.type == DISK_TYPE_SATAPI){
          printf("VFS: Chdir operation not supported on ISO9660 (disk %d)\n", disk_no);
          return -1;
     }else if(disk.type == DISK_TYPE_AHCI_SATA){
          return f_chdir(path) ? 0 : -1;
     }else{
          return -1;
     }
}




int vfs_getcwd(int disk_no, char *buff, int len){
    Disk disk = disks[disk_no];
     if(disk.type == DISK_TYPE_SATAPI){
          printf("VFS: Getcwd operation not supported on ISO9660 (disk %d)\n", disk_no);
          return -1;
     }else if(disk.type == DISK_TYPE_AHCI_SATA){
          return f_getcwd(buff, len) ? 0 : -1;
     }else{
          return -1;
     }
}


int vfs_getfree(int disk_no, char *path){
    Disk disk = disks[disk_no];
     if(disk.type == DISK_TYPE_SATAPI){
          printf("VFS: Getfree operation not supported on ISO9660 (disk %d)\n", disk_no);
          return -1;
     }else if(disk.type == DISK_TYPE_AHCI_SATA){
          return 0;
     }else{
          return -1;
     }
}

int vfs_getlabel(int disk_no, char *path, char* label, void *vsn){
    Disk disk = disks[disk_no];
     if(disk.type == DISK_TYPE_SATAPI){
          printf("VFS: Getlabel operation not supported on ISO9660 (disk %d)\n", disk_no);
          return -1;
     }else if(disk.type == DISK_TYPE_AHCI_SATA){
          return 0;
     }else{
          return -1;
     }
}


int vfs_setlabel(int disk_no, char *label){
    Disk disk = disks[disk_no];
     if(disk.type == DISK_TYPE_SATAPI){
          printf("VFS: Setlabel operation not supported on ISO9660 (disk %d)\n", disk_no);
          return -1;
     }else if(disk.type == DISK_TYPE_AHCI_SATA){
          return 0;
     }else{
          return -1;
     }
}


int vfs_forward(int disk_no){
    Disk disk = disks[disk_no];
     if(disk.type == DISK_TYPE_SATAPI){
          printf("VFS: Forward operation not supported on ISO9660 (disk %d)\n", disk_no);
          return -1;
     }else if(disk.type == DISK_TYPE_AHCI_SATA){
          return 0;
     }else{
          return -1;
     }
}


int vfs_expand(int disk_no){
    Disk disk = disks[disk_no];
     if(disk.type == DISK_TYPE_SATAPI){
          printf("VFS: Expand operation not supported on ISO9660 (disk %d)\n", disk_no);
          return -1;
     }else if(disk.type == DISK_TYPE_AHCI_SATA){
          return 0;
     }else{
          return -1;
     }
}



int vfs_get_fsize(int disk_no, void *fp) {
    Disk disk = disks[disk_no];
    switch(disk.type){
        case DISK_TYPE_SATAPI:
            return iso9660_get_fsize(fp);
            break;
        case DISK_TYPE_AHCI_SATA:
            return f_size(fp);
            break;
        default:
            printf("VFS: Unsupported disk type %d for get_fsize on disk %d\n", disk.type, disk_no);
            return -1;
    }
}




const char* vfs_error_string(int result){

    switch(result){
        case 0:
            return "OK";
        case -1:
            return "DISK_ERR";
    }

    return "UNKNOWN_ERROR";
}

uint64_t vfs_listdir(int disk_no, char *path){
    if(disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_AHCI_SATA){

        FAT32_DIR dir;

        FAT32_DIRENT entry;

        if (!f_opendir(&dir, path)){
            return -1;
        }


        while (f_readdir(&dir, &entry))
        {
            printf("  name:%s  size:%u  cluster:%u  attr:%02X\n",
                entry.name,
                entry.size,
                entry.first_cluster,
                entry.attr);
        }

        f_closedir(&dir);
    }

    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Listdir operation not supported on ISO9660 (disk %d)\n", disk_no);
        return -1;
    }
    return -1;
}


bool vfs_test(int disk_no, uint32_t lba, VFS_TYPE type){

    printf("================ VFS TEST Start ======================\n");

    if(disk_no >= disk_count || !disks) return false;

    Disk disk = disks[disk_no];

    if(vfs_mount(disk_no, lba, type) != 0){
        printf("Failed to mount\n");
    }else{
        printf("Successfully Mount Disk at LBA %d\n", lba);
    }

    FAT32_FILE* fp = malloc(sizeof(FAT32_FILE));
    if(!fp){
        free(fp);
        return false;
    }
    memset((void *)fp, 0, sizeof(FAT32_FILE));
    printf("first_cluster: %d\n \
            current cluster: %d\n \
            size: %d\n \
            pos: %d\n \
            parent_cluster: %d\n \
            dir_entry_cluster: %d\n \
            dir_entry_offset: %d\n \
            name: %s\n \
            mode: %d\n", \
            fp->first_cluster, \
            fp->current_cluster, \
            fp->size, \ 
            fp->pos, \
            fp->parent_cluster, \
            fp->dir_entry_cluster, \
            fp->dir_entry_offset, \
            fp->name, fp->mode );


    // void *testfile = vfs_open(disk_no, "/testfile.txt", VFS_CREATE_ALWAYS | VFS_WRITE);

   printf("================ VFS TEST END ======================\n");
}










