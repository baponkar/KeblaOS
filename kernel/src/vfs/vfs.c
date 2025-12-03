
/*
Virtual File System

References:
    https://wiki.osdev.org/VFS

*/

#include "../driver/disk/disk.h"                

#include "../fs/FatFs-R.0.16/source/ff.h"
#include "../fs/FatFs-R.0.16/source/diskio.h"

#include "../fs/fatfs_wrapper.h"
#include "../fs/iso9660.h"

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/limit.h"
#include "../lib/errno.h"

#include "vfs.h"



#define SECTOR_SIZE 512



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
    
    if (disk_no >= disk_count){
        printf("VFS: Invalid disk number %d\n", disk_no);
        return -1;
    } 

    if(!disks){
        printf("VFS: disks is NULL\n");
        return -1;
    }

    Disk disk = disks[disk_no];
    if (disk.type == DISK_TYPE_SATAPI) {
        return iso9660_init(disk_no);
    } else if (disk.type == DISK_TYPE_AHCI_SATA) {
        return fatfs_init(disk_no);
    } else {
        printf("VFS: Unsupported disk type %d for init on disk %d\n", disk.type, disk_no);
        return -1;
    }
    
    return 0;
}


int vfs_disk_status(int disk_no){
    if(!kebla_disk_status(disk_no)){
        printf(" VFS: Disk %d status check failed!\n", disk_no);
        return -1;
    }
    return 0;
}


int vfs_mount(int disk_no){
    if(disk_no >= disk_count) {
        printf("VFS: Invalid disk number %d\n", disk_no);
        return -1;
    }

    if(!disks) {
        printf("VFS: Disks not initialized!\n");
        return -1;
    }

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_mount(disk_no);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_mount(disk_no);
    }else{
        printf("VFS: Unsupported disk type %d for mount on disk %d\n", disk.type, disk_no);
        return -1;
    }

    return 0;
}

int vfs_unmount(int disk_no){
    if(disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_unmount(disk_no);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_unmount(disk_no);
    }

    return -1;
}


int vfs_mkfs(int disk_no, VFS_TYPE fs_type){

    if(disk_no >= disk_count){
        printf("VFS: Invalid disk number %d\n", disk_no);
        return -1;
    } 

    if(fs_type == VFS_FAT12){
        return fatfs_mkfs(disk_no, FM_FAT);
    }else if(fs_type == VFS_FAT16){
        return fatfs_mkfs(disk_no, FM_FAT);
    }else if(fs_type == VFS_FAT32){
        printf(" Creating FAT32 Filesystem in Disk %d\n", disk_no);
        return fatfs_mkfs(disk_no, FM_FAT32 | FM_SFD);
    }else if(fs_type == VFS_EXFAT){
        return fatfs_mkfs(disk_no, FM_EXFAT);
    }else{
        printf("VFS: Unsupported filesystem type %d for mkfs on disk %d\n", fs_type, disk_no);
        return -1;
    }

    return 0;
}


#if FF_MULTI_PARTITION
int vfs_fdisk(int disk_no, void *ptbl, void* work){
    if(disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];

    switch(disk.type){
        case DISK_TYPE_AHCI_SATA:
            return fatfs_fdisk(disk_no, ptbl, work);
            break;
        default:
            printf("VFS: Unsupported disk type %d for fdisk on disk %d\n", disk.type, disk_no);
            return -1;
    }
}
#endif

int vfs_setcp(int disk_no, int cp){
    if(disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];

    switch(disk.type){
        case DISK_TYPE_AHCI_SATA:
            return fatfs_setcp(cp);
            break;
        default:
            printf("VFS: Unsupported disk type %d for setcp on disk %d\n", disk.type, disk_no);
            return -1;
    }
}

#if FF_USE_STRFUNC

int vfs_putc(int disk_no, void *fp, char c){
    if(disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];

    switch(disk.type){
        case DISK_TYPE_AHCI_SATA:
            return fatfs_putc(fp, c);
            break;
        default:
            printf("VFS: Unsupported disk type %d for putc on disk %d\n", disk.type, disk_no);
            return -1;
    }
}


int vfs_puts(int disk_no, char *str, void *cp){
    if(disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];

    switch(disk.type){
        case DISK_TYPE_AHCI_SATA:
            return fatfs_puts(str, cp);
            break;
        default:
            printf("VFS: Unsupported disk type %d for puts on disk %d\n", disk.type, disk_no);
            return -1;
    }
}


char *vfs_gets(int disk_no, char *buff, int len, void *fp){
    if(disk_no >= disk_count) return NULL;

    Disk disk = disks[disk_no];

    switch(disk.type){
        case DISK_TYPE_AHCI_SATA:
            return fatfs_gets(buff, len, fp);
            break;
        default:
            printf("VFS: Unsupported disk type %d for gets on disk %d\n", disk.type, disk_no);
            return NULL;
    }
}

#endif

#if FF_PRINT_LLI
int vfs_printf(int disk_no, void *fp, char *str){
    if(disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];

    switch(disk.type){
        case DISK_TYPE_AHCI_SATA:
            return fatfs_printf(fp, str);
            break;
        default:
            printf("VFS: Unsupported disk type %d for printf on disk %d\n", disk.type, disk_no);
            return -1;
    }
}
#endif

void *vfs_open(int disk_no, char *path, int mode){
    if(disk_no >= disk_count) return NULL;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_open(disk_no, path, mode);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_open(path, mode);
    }

    return NULL;
}


int vfs_close(int disk_no, void *fp){
    if(!fp) return -1;

    Disk disk = disks[disk_no];

    switch(disk.type){
        case DISK_TYPE_SATAPI:
            return iso9660_close(fp);
            break;
        case DISK_TYPE_AHCI_SATA:
            return fatfs_close(fp);
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
            return fatfs_read(fp, buff, size);
            break;
        default:
            printf("VFS: Unsupported disk type %d for read on disk %d\n", disk.type, disk_no);
            return -1;
    }
}

int vfs_write(int disk_no, void *fp, char *buff, int filesize){
    Disk disk = disks[disk_no];
    switch(disk.type){
        case DISK_TYPE_SATAPI:
            // ISO9660 is read-only
            printf("VFS: Write operation not supported on ISO9660 (disk %d)\n", disk_no);
            return -1;
            break;
        case DISK_TYPE_AHCI_SATA:
            return fatfs_write(fp, buff, filesize);
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
            return fatfs_lseek(fp, offset);
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
            return fatfs_truncate(fp);
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
            return fatfs_sync(fp);
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
        return fatfs_opendir(path);
    }

    return NULL;
}



int vfs_closedir(int disk_no, void *dp){
    if(!dp) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_closedir(dp);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_closedir(dp);
    }

    return -1;
}

int vfs_readdir(int disk_no, void *dp, void *fno){
    if(!dp | !fno) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_readdir(dp, fno);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_readdir(dp, fno);
    }

    return -1;
}




#if FF_USE_FIND
int vfs_findfirst(int disk_no, void *dp, void *fno, char *path, char *pattern){
    if(!dp | !fno | !path | !pattern) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Findfirst operation not supported on ISO9660 (disk %d)\n", disk_no);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_findfirst(dp, fno, path, pattern);
    }

    return -1;
}

int vfs_findnext(int disk_no, void *dp, void *fno){
    if(!dp | !fno) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Findnext operation not supported on ISO9660 (disk %d)\n", disk_no);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_findnext(dp, fno);
    }

    return -1;
}
#endif


int vfs_mkdir(int disk_no, char *path){
    if(!path) return -1;
    if(!disks) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Mkdir operation not supported on ISO9660 (disk %d)\n", disk_no);
        return -1;
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_mkdir(path);
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
        return fatfs_unlink(path);
    }

    return -1;
}

int vfs_rename(int disk_no, char *old_path, char *new_path){
    if(!old_path | !new_path) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        printf("VFS: Rename operation not supported on ISO9660 (disk %d)\n", disk_no);
        return -1;
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_rename(old_path, new_path);
    }

    return -1;
}

int vfs_stat(int disk_no, char *path, void *fno){
    if(!path | !fno) return -1;

    Disk disk = disks[disk_no];

    if(disk.type == DISK_TYPE_SATAPI){
        return iso9660_stat(disk_no, path, fno);
    }else if(disk.type == DISK_TYPE_AHCI_SATA){
        return fatfs_stat(path, fno);
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
        return fatfs_chmod(path, attr, mask);
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
        return fatfs_utime(path, fno);
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
          return fatfs_chdir(path);
     }else{
          return -1;
     }
}

int vfs_chdrive(int disk_no, char *path){
    Disk disk = disks[disk_no];
     if(disk.type == DISK_TYPE_SATAPI){
          printf("VFS: Chdrive operation not supported on ISO9660 (disk %d)\n", disk_no);
          return -1;
     }else if(disk.type == DISK_TYPE_AHCI_SATA){
          return fatfs_chdrive(path);
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
          return fatfs_getcwd(buff, len);
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
          return fatfs_getfree(path);
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
          return fatfs_getlabel(path, label, vsn);
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
          return fatfs_setlabel(label);
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
          return fatfs_forward();
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
          return fatfs_expand();
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
            return fatfs_get_fsize(fp);
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


void vfs_test(int disk_no){
    if(disk_no >= disk_count) return;

    if(!disks) return;

    Disk disk = disks[disk_no];
    
    // VFS Initializatiion
   if(vfs_init(disk_no) != 0){
        printf(" Error in initializing VFS in disk %d\n", disk_no);
        return;
   }
   printf(" Successfully initialized VFS in disk %d\n", disk_no);

   // Making FAT32 Filesystem
   if(vfs_mkfs(disk_no, VFS_FAT32) != 0){
        printf(" Error to create FAT32 Filesystem in disk %d\n", disk_no);
        return;
   }
   printf(" Successfully created FAT32 Filesystem in Disk %d\n", disk_no);

   // Mounting the Disk
   if(vfs_mount(disk_no) != 0){
        printf(" Error to mount disk %d\n", disk_no);
        return;
   }
   printf(" Successfully mount Disk %d\n", disk_no);

   // Creating a testfile test.txt
   int flag = FA_CREATE_ALWAYS | FA_WRITE ;
   void *test_file = vfs_open(disk_no, "1:/test.txt", flag);

   if(!test_file){
        printf(" Failed to create test.txt file\n");
        return;
   }
   printf(" Successfully opened test.txt\n");

   const char *text = "Hello from FATFS.\n";

   char buff[256];
   if(vfs_write(disk_no, test_file, text, strlen(text)) != 0){
        printf(" failed to write into test.txt\n");
        return;
   }
   printf(" successfully written in test.txt\n");

   if(vfs_close(disk_no, test_file) != 0){
        printf(" failed to close the file test.txt\n");
        return;
   }
   printf(" Successfully closed the file test.txt\n");

   // reopen
   int flag_1 = FA_READ;
   test_file = vfs_open(disk_no, "1:/test.txt", flag_1);
   if(!test_file){
        printf(" failed to reopen the test file\n");
        return;
   }

   memset(buff, 0, sizeof(buff));
   if(vfs_read(disk_no, test_file, buff, 255) != 0){
        printf(" Failed to read the test file\n");
        return;
   }
   buff[255] = '\0';
   printf(" Successsfully read the test file: %s\n", buff);
}










