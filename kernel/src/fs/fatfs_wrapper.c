
// https://elm-chan.org/fsw/ff/


#include "FatFs-R.0.16/source/ff.h"
#include "FatFs-R.0.16/source/diskio.h"
#include "FatFs-R.0.16/source/ffconf.h"

#include "../lib/stdlib.h"
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"
#include "../lib/time.h"

#include "../memory/kheap.h"

#include "fatfs_wrapper.h"


FATFS *fs_array[FF_VOLUMES] = {NULL};

/* Error code to string conversion */
const char* fatfs_error_string(FRESULT result)
{
    if (result < sizeof(error_strings) / sizeof(error_strings[0])) {
        return error_strings[result];
    }
    return "Unknown error";
}

// This function is called by FatFs to get current time
DWORD get_fattime(void)
{
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    if (!tm) {
        // Fallback: 1 Jan 2025 00:00:00
        return ((DWORD)(2025 - 1980) << 25)
             | ((DWORD)1 << 21)
             | ((DWORD)1 << 16);
    }

    DWORD year  = (DWORD)(tm->tm_year + 1900 - 1980);
    DWORD month = (DWORD)(tm->tm_mon + 1);
    DWORD day   = (DWORD)(tm->tm_mday);
    DWORD hour  = (DWORD)(tm->tm_hour);
    DWORD min   = (DWORD)(tm->tm_min);
    DWORD sec   = (DWORD)(tm->tm_sec / 2);

    return (year  << 25)
         | (month << 21)
         | (day   << 16)
         | (hour  << 11)
         | (min   << 5)
         | (sec);
}


// Initialize FATFS on the given physical disk 
int fatfs_init(int pdrv){
    return disk_initialize(pdrv);
}

int fatfs_disk_status(int disk_no){
    return disk_status (disk_no);
}

// Making FATFS on the given logical drive
int fatfs_mkfs(int ld, int fs_type) {
    if(ld < 0) return -1;
    
    MKFS_PARM opt;          
    opt.fmt = fs_type;
    opt.n_fat = 1;
    opt.align = 0;
    opt.n_root = 0;
    opt.au_size = 0;
    
    // Try larger buffer
    size_t buffer_size = 16 * 1024;  // 16 KB
    BYTE *work = (BYTE *)malloc(buffer_size);

    if(!work) {
        printf("FATFS: Failed to allocate any work buffer\n");
        return -1;
    }

    memset(work, 0, buffer_size);

    char root_path[4];
    snprintf(root_path, sizeof(root_path), "%d:", ld);
    
    FRESULT res = f_mkfs(root_path, &opt, work, buffer_size);
    
    free(work);
    
    if(res != FR_OK) {
        printf("FATFS: MKFS failed with error: %s\n", fatfs_error_string(res));
        return -1;
    }
    
    return (int)res;
}

// Mounting FATFS on the given logical drive
int fatfs_mount(int ld){
    if(!fs_array[ld]){
        fs_array[ld] = (FATFS *)malloc(sizeof(FATFS));
    }
    memset(fs_array[ld], 0, sizeof(FATFS));

    char path[16];
    memset(path, 0, sizeof(path));
    sprintf(path, "%d:", ld);

    BYTE mount_opt = 1;  // 1 = immediate mount with detection, 0 = delay mount

    FRESULT res = f_mount(fs_array[ld], (const TCHAR*) path, mount_opt);

    if(res != FR_OK){
        printf("FATFS: Mounting failed on drive %d with error: %s\n", ld, fatfs_error_string(res));
    }

    // printf("FATFS: Successfully mounted on LDRV: %d\n", ld);
    return (int) res;
}

// Unmounting FATFS from the given logical drive
int fatfs_unmount(int ld){
    char path[16];
    memset(path, 0, sizeof(path));
    sprintf(path, "%d:", ld);
    const TCHAR* root_path = (const TCHAR*) path;

    return f_mount(NULL, root_path, 0);
}


#if FF_MULTI_PARTITION
PARTITION VolToPart[FF_VOLUMES] = {
    {1, 1},     // "0:" ==> 1st partition in physical drive 1
    {1, 2}      // "1:" ==> 2nd partition in physical drive 1
};
#endif


#if FF_MULTI_PARTITION
// Divide a physical drive into some partitions
int fatfs_fdisk(int physical_disk_no, void *ptbl, void* work){
    return f_fdisk((BYTE) physical_disk_no, (const LBA_t *) ptbl, work);
}
#endif

// Set current code page
int fatfs_setcp(int cp){
    f_setcp((WORD) cp);
}

#if FF_USE_STRFUNC
// Put a Char in fp
int fatfs_putc(void *fp, char c){
    return f_putc((TCHAR) c, (FIL*) fp);
}

// Put String into file
int fatfs_puts(char *str, void *cp){
    return f_puts ((const TCHAR*) str, (FIL*) cp);	
}

//Get a string from the file
char *fatfs_gets(char *buff, int len, void *fp){
    return (char *)f_gets ((TCHAR*) buff, len, (FIL*)fp);
}
#endif

#if FF_PRINT_LLI
// Printing file content
int fatfs_printf(void *fp, char *str){
    // return f_printf ((FIL*) fp, (const TCHAR*) str, ...);
}
#endif


// open a file given in logical path with mode
void *fatfs_open(char *path, int mode) {

    if (!path) return NULL;

    FIL *fp = (FIL *)malloc(sizeof(FIL));  // dynamically allocate
    if (!fp) return NULL;

    FRESULT res = f_open(fp, (const TCHAR*) path, (BYTE) mode);
    if (res != FR_OK) {
        printf("Failed to open %s with error code %d\n", path, mode);
        free(fp);
        return NULL;
    }

    return (void *)fp;
}


// Close the file
int fatfs_close(void *fp){
    if(!fp) return -1;
    FRESULT res = f_close((FIL*) fp);
    free(fp);

    return res;
}

// reading file and put contents in buffer
int fatfs_read(void *fp, char *buff, int size){
    if(!fp) return -1;
    
    UINT br;

    int res = f_read ((FIL*) fp, (void *)buff, size, &br);

    return res == FR_OK ? br : -1;
}


// writing buffer content into file
int fatfs_write(void *fp, char *buff, int filesize){
    if(!fp || !buff) return -1;
    UINT bw;

    return f_write ((FIL*) fp, (void*) buff, filesize, &bw) == FR_OK ? bw: -1;
}

// Change Current pointer
int fatfs_lseek(void *fp, int offset){
    return f_lseek ((FIL*) fp, (FSIZE_t) offset);	
}

// It shortens the file — everything after the current file position is deleted (removed from the file).
int fatfs_truncate(void *fp){
    return f_truncate ((FIL*) fp);
}

// The f_sync() function forces any cached data of the file to be physically written to the disk.
int fatfs_sync(void *fp){
   return f_sync((FIL*)fp);
}

// open a directory
void *fatfs_opendir(char *path){
    DIR dp;
    if(f_opendir((DIR*) &dp, (const TCHAR*) path) == FR_OK){
        return (void *)&dp;
    }
    return NULL;
}

// close the directory
int fatfs_closedir(void *dp){
    return f_closedir(dp);
}

// 
int fatfs_readdir(void *dp, void *fno){
    return f_readdir((DIR*) dp, (FILINFO*) fno);
}


#if FF_USE_FIND
int fatfs_findfirst(void *dp, void *fno, char *path, char *pattern){
    return f_findfirst((DIR*)dp, (FILINFO*)fno, (const TCHAR*)path, (const TCHAR*)pattern);
}

int fatfs_findnext(void *dp, void *fno){
    return f_findnext ((DIR*) dp, (FILINFO*) fno);
}
#endif

int fatfs_mkdir(char *path){
    return f_mkdir((const TCHAR*) path) == FR_OK ? 0 : -1;
}

int fatfs_unlink(char *path){
    return f_unlink((const TCHAR*) path);
}

int fatfs_rename(char *old_path, char *new_path){
    return f_rename (old_path, new_path);
}

int fatfs_stat(char *path){
    FILINFO* fno = (FILINFO*) malloc(sizeof(FILINFO));
    return f_stat((const TCHAR*) path, (FILINFO*) fno);
}

int fatfs_chmod(char *path, int attr, int mask){
    return f_chmod ((const TCHAR*) path, (BYTE) attr, (BYTE) mask);
}

int fatfs_utime(char *path, void *fno){
    return f_utime((const TCHAR*) path, (const FILINFO*) fno);
}

// Change current directory
int fatfs_chdir(char *path){
   return f_chdir((const TCHAR*) path);
}

// Change current drive
int fatfs_chdrive(char *path){
    return f_chdrive((const TCHAR*) path);	
}

// In fatfs_getcwd, add bounds checking
int fatfs_getcwd(char *buff, int len) {
    // printf("fatfs_getcwd: buff=%p, len=%d\n", buff, len);
    int result = f_getcwd((TCHAR*) buff, (UINT) len);
    // Add a null terminator check
    if (result == 0 && len > 0) {
        buff[len-1] = '\0'; // Ensure null termination
    }
    return result;
}

//  Get number of free clusters on the drive
int fatfs_getfree(char *path ){
    DWORD nclast;
    FATFS **fatfs = (FATFS **)malloc(sizeof(FATFS*));
    if(!fatfs) return -1;

    FRESULT res = f_getfree ((const TCHAR*) path, (DWORD*) &nclast, fatfs);

    free(fatfs);

    return res;
}

// Get volume label
int fatfs_getlabel(char *path, char* label, void *vsn){
    return f_getlabel((const TCHAR*) path, (TCHAR*) label, vsn);
}

// Set volume label
int fatfs_setlabel(char *label){
    return f_setlabel(label);
}

int fatfs_forward(){
    // f_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);
}

int fatfs_expand(){
    // f_expand (FIL* fp, FSIZE_t fsz, BYTE opt);
}

int fatfs_get_fsize(void *fp){
    if(!fp) return -1;

    FFOBJID	obj = ((FIL*)fp)->obj;

    return (int)obj.objsize;
}

int fatfs_listdir(char *path){
    DIR dir;
    FILINFO fno;
    FRESULT res;

    res = f_opendir(&dir, path);
    if(res != FR_OK){
        printf("FATFS: Failed to open directory %s with error %s\n", path, fatfs_error_string(res));
        return -1;
    }

    printf("Listing directory: %s\n", path);
    for(;;){
        res = f_readdir(&dir, &fno);
        if(res != FR_OK || fno.fname[0] == 0) break;    // Break on error or end of dir

        if(fno.fattrib & AM_DIR){
            printf("<DIR>  %s\n", fno.fname);
        } else {
            printf("       %s  %lu bytes\n", fno.fname, fno.fsize);
        }
    }

    f_closedir(&dir);
    return 0;
}



void fatfs_test(int disk_no){

    printf("[FATFS] Starting FatFS Testing....\n");

    if(fatfs_init(disk_no) == 0){
        printf(" Successfully initialized Disk %d\n", disk_no);
    }

    if(fatfs_mkfs(disk_no, FM_FAT32 | FM_SFD) == 0){
        printf(" Successfully made FAT32 Filesystem in Disk %d\n", disk_no);
    }

    if(fatfs_mount(disk_no) == 0){
        printf(" Successfully mounted Disk %d\n", disk_no);
    }

    int flag = FA_CREATE_ALWAYS | FA_WRITE ;
    void *test_file = fatfs_open("1:/testfile.txt", flag);
    if(test_file != NULL){
        printf(" Successfully open 1:/testfile.txt\n");
    }

    const char *text = "Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! Hello from FATFS! ";
    if(test_file != NULL){
        int bytes_written = fatfs_write(test_file, text, strlen(text));
        if(bytes_written > 0){
            printf(" Successfully wrote %d bytes in 1:/testfile.txt\n", bytes_written);
        } else {
            printf(" Write failed!\n");
        }
    }

    if(fatfs_sync(test_file) == 0){
        printf(" Successfully synced file\n");
    }

    if(fatfs_close(test_file) == 0){
        printf(" Successfully Closed 1:/testfile.txt\n");
    }

    test_file = fatfs_open("1:/testfile.txt", FA_READ);
    if(test_file != NULL){
        printf(" Successfully opened 1:/testfile.txt again\n");
    }

    if(fatfs_lseek(test_file, 4) == 0){
        printf(" Successfully change pointer offset 4\n");
    }

    char read_buff[128];
    if(fatfs_read(test_file, read_buff, 127) == 0){
        read_buff[127] = '\0';
        printf(" Reading Successfull: %s\n", read_buff);
    }

    if(fatfs_close(test_file) == 0){
        printf(" Successfully Closed 1:/testfile.txt\n");
    }

    char cwd_buf[25];
    memset(cwd_buf, 0, strlen(cwd_buf));
    if(fatfs_getcwd(cwd_buf, sizeof(cwd_buf)) == 0){
        cwd_buf[24] = '\0';
        printf(" Current Working Directory: %s\n", cwd_buf);
    }

    if(fatfs_unmount(disk_no) == 0){
        printf(" Successfully Unmount The Disk\n");
    }

    printf("[FatFS] Successfully Completed FatFS\n");
}


#if FF_MULTI_PARTITION
void fatfs_test_1(int disk_no) {
    
    int logical_drive_1 = 0;                      // First logical drive
    int logical_drive_2 = 1;                      // Second logical drive

    int partition_1 = 1;                          // First partition
    int partition_2 = 2;                          // Second partition

    VolToPart[logical_drive_1].pd = disk_no;      // Physical drive number
    VolToPart[logical_drive_1].pt = partition_1;  // Partition 1

    VolToPart[logical_drive_2].pd = disk_no;      // Physical drive number  
    VolToPart[logical_drive_2].pt = partition_2;  // Partition 2

    size_t buffer_size = 65536;  // 64KB
    BYTE* work = (BYTE*)malloc(buffer_size);        // 64K buffer
    if(!work) {
        printf("Failed to allocate %zu bytes for formatting buffer\n", buffer_size);
        return;
    }
    memset(work, 0, buffer_size); 

    // 256MB sectors for the 1st partition and the remaining for the 2nd partition
    uint64_t part_1_sectors = (256 * 1024 * 1024) / 512;

    LBA_t plist[] = {part_1_sectors, 100};      // 100 MB space for the 1st partition and the remaining space for the 2nd partition

    int res = f_fdisk(disk_no, plist, work);    // Divide the physical drive 1

    if(res != FR_OK){      
        printf("FATFS: FDISK failed on physical drive %d with error %s\n", disk_no, fatfs_error_string(res));
        free(work);
        return;
    }
    printf("FATFS: FDISK succeeded on physical drive %d\n", disk_no);

    free(work);
    work = NULL;

    res = fatfs_mkfs(logical_drive_1, FM_FAT32);      // Create FAT volume on the logical drive 0
    if(res != FR_OK){      
        printf("FATFS: MKFS failed on logical drive 0 with error %s\n", fatfs_error_string(res));
        return;
    }


    res = fatfs_mkfs(logical_drive_2, FM_FAT32);        // Create FAT volume on the logical drive 1
    if(res != FR_OK){      
        printf("FATFS: MKFS failed on logical drive 1 with error %s\n", fatfs_error_string(res));
        return;
    }


    res = fatfs_mount(logical_drive_1);                  // mount logical drive 0
    if(res != FR_OK){      
        printf("FATFS: mounting failed on logical drive 0 with error %s\n", fatfs_error_string(res));
        return;
    }


    res = fatfs_mount(logical_drive_2);                    // mount logical drive 1
    if(res != FR_OK){      
        printf("FATFS: mounting failed on logical drive 1 with error %s\n", fatfs_error_string(res));
        return;
    }


    //-----------------------------------------------------------------------------------------------

    const char *msg0 = "This is FAT32 Partition 0";
    void *file0 = fatfs_open("0:/part0.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (file0) {
        fatfs_write(file0, (char*)msg0, strlen(msg0));
        printf("Successfully wrote unique files to both partitions.\n");
    }
    fatfs_close(file0);
    printf("\n--- Listing 0: ---\n");
    fatfs_listdir("0:/");
    fatfs_unmount(0);

    const char *msg1 = "This is FAT32 Partition 1";
    fatfs_chdrive("1:");
    void *file1 = fatfs_open("1:/part1.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if ( file1) {
        fatfs_write(file1, (char*)msg1, strlen(msg1));
        printf("Successfully wrote unique files to both partitions.\n");
    }
    fatfs_close(file1);
    printf("\n--- Listing 1: ---\n");
    fatfs_listdir("1:/");
    fatfs_unmount(1);
}
#endif

















