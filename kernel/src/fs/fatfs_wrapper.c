

#include "FatFs-R0.15b/source/ff.h"
#include "FatFs-R0.15b/source/diskio.h"

#include "../lib/stdlib.h"
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"
#include "../lib/time.h"

#include "fatfs_wrapper.h"


FATFS *fs;

/* Error code to string conversion */
const char* fatfs_error_string(FRESULT result)
{
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
    
    if (result < sizeof(error_strings) / sizeof(error_strings[0])) {
        return error_strings[result];
    }
    return "Unknown error";
}

int fatfs_init(int disk_no){
    fs = (FATFS *)malloc(sizeof(FATFS));
    if(!fs){
        printf("memory allocation for fs is failed!\n");
        return -1;
        free(fs);
    }
    memset(fs, 0, sizeof(FATFS));

    return (int) disk_initialize (disk_no);
}

int fatfs_disk_status(int disk_no){
    return disk_status (disk_no);
}

int fatfs_mkfs(int disk_no, int fs_type){
    MKFS_PARM opt;          

    opt.fmt = fs_type;      // FM_FAT, FM_FAT32, FM_EXFAT, FM_ANY, FM_SFD
    opt.n_fat  = 1;         // One FAT
    opt.align  = 0;         // Default alignment
    opt.n_root = 0;         // Not used for FAT32
    opt.au_size= 0;         // Auto cluster size
    
    BYTE work[4096];        // 4K buffer, enough for 512-byte sectors

    char root_path[16];
    sprintf(root_path, "%d:",disk_no);
    
    return f_mkfs(root_path, &opt, work, sizeof(work));
}

int fatfs_mount(int disk_no){
    if(!fs){
        fs = (FATFS *)malloc(sizeof(FATFS));
    }
    memset(fs, 0, sizeof(FATFS));
    
    char path[16];
    sprintf(path, "%d:", disk_no);
    const TCHAR* root_path = (const TCHAR*) path;

    BYTE mount_opt;

    FRESULT mount_res = f_mount (fs, root_path, mount_opt);

    return mount_res;
}

int fatfs_unmount(int disk_no){
    char path[16];
    sprintf(path, "%d:", disk_no);
    const TCHAR* root_path = (const TCHAR*) path;

    return f_mount (NULL, root_path, 0);
}

#if FF_MULTI_PARTITION

PARTITION VolToPart[FF_VOLUMES] = {
        {0, 1},    /* "0:" ==> 1st partition in physical drive 0 */
        {0, 2},    /* "1:" ==> 2nd partition in physical drive 0 */
        {1, 0}     /* "2:" ==> Physical drive 1 as removable drive */
};

// Divide a physical drive into some partitions
int fatfs_fdisk(int disk_no, void *ptbl, void* work){
    // return f_fdisk((BYTE) disk_no, (const LBA_t *) ptbl, work);
}
#endif

// Set current code page
int fatfs_setcp(int cp){
    f_setcp((WORD) cp);
}

// Put a Char in fp
int fatfs_putc(void *fp, char c){
    return f_putc((TCHAR) c, (FIL*) fp);
}

// Put String into file
int fatfs_puts(char *str, void *cp){
    return f_puts ((const TCHAR*) str, (FIL*) cp);	
}

// Printing file content
int fatfs_printf(void *fp, char *str){
    // return f_printf ((FIL*) fp, (const TCHAR*) str, ...);
}

//Get a string from the file
char *fatfs_gets(char *buff, int len, void *fp){
    return (char *)f_gets ((TCHAR*) buff, len, (FIL*)fp);
}

// open a file
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

    return f_read(fp, (void *)buff, size, &br);
}


// writing buffer content into file
int fatfs_write(void *fp, char *buff, int filesize){
    if(!fp | !buff) return -1;
    UINT bw;

    return f_write ((FIL*) fp, (void*) buff, filesize, &bw);
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
int fatfs_sync(void * fp){
   return f_sync((FIL*) fp);
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

int fatfs_findfirst(void *dp, void *fno, char *path, char *pattern){
    return f_findfirst((DIR*)dp, (FILINFO*)fno, (const TCHAR*)path, (const TCHAR*)pattern);
}


int fatfs_findnext(void *dp, void *fno){
    return f_findnext ((DIR*) dp, (FILINFO*) fno);
}


int fatfs_mkdir(char *path){
    return f_mkdir (path);
}

int fatfs_unlink(char *path){
    return f_unlink((const TCHAR*) path);
}

int fatfs_rename(char *old_path, char *new_path){
    return f_rename (old_path, new_path);
}

int fatfs_stat(char *path, void *fno){
    return f_stat((const TCHAR*) path, (FILINFO*) fno);
}

int fatfs_chmod(char *path, int attr, int mask){
    return f_chmod ((const TCHAR*) path, (BYTE) attr, (BYTE) mask);
}

int fatfs_utime(char *path, void *fno){
    return f_utime((const TCHAR*) path, (const FILINFO*) fno);
}

int fatfs_chdir(char *path){
   return f_chdir((const TCHAR*) path);
}

int fatfs_chdrive(char *path){
    return f_chdrive((const TCHAR*) path);	
}

int fatfs_getcwd(char *buff, int len){
    return f_getcwd((TCHAR*) buff, (UINT) len);
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

int fatfs_getlabel(char *path, char* label, void *vsn){
    return f_getlabel((const TCHAR*) path, (TCHAR*) label, vsn);
}

int fatfs_setlabel(char *label){
    return f_setlabel(label);
}

int fatfs_forward(){
    // f_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);
}

int fatfs_expand(){
    // f_expand (FIL* fp, FSIZE_t fsz, BYTE opt);
}

void fatfs_test(int disk_no){

    if(fatfs_init(disk_no) == FR_OK){
        printf("Successfully initialized Disk %d\n", disk_no);
    }

    if(fatfs_disk_status(disk_no) == FR_OK){
        printf("Disk Status %d for Disk-%d\n", FR_OK, disk_no);
    }

    if(fatfs_mkfs(disk_no, FM_FAT32) == FR_OK){
        printf("Successfully made FAT32 Filesystem in Disk %d\n", disk_no);
    }

    if(fatfs_mount(disk_no) == FR_OK){
        printf("Successfully mounted Disk %d\n", disk_no);
    }

    int flag = FA_CREATE_ALWAYS | FA_WRITE ;
    void *test_file = fatfs_open("0:/testfile.txt", flag);
    if(test_file != NULL){
        printf("Successfully open 0:/testfile.txt\n");
    }

    const char text[] = "Hello from FATFS\r\n";
    if(test_file != NULL){
        if(fatfs_write(test_file, text, strlen(text)) == FR_OK){
            printf("Successfully Write in 0:/testfile.txt\n");
        }
    }

    if(fatfs_sync(test_file) == FR_OK){
        printf("Successfully synced file\n");
    }

    if(fatfs_close(test_file) == FR_OK){
        printf("Successfully Closed 0:/testfile.txt\n");
    }

    test_file = fatfs_open("0:/testfile.txt", FA_READ);
    if(test_file != NULL){
        printf("Successfully opened 0:/testfile.txt again\n");
    }

    if(fatfs_lseek(test_file, 4) == FR_OK){
        printf("Successfully change pointer offset 4\n");
    }

    char read_buff[26];
    if(fatfs_read(test_file, read_buff, 25) == FR_OK){
        read_buff[25] = '\0';
        printf("Reading Successfull: %s\n", read_buff);
    }

    char cwd_buf[256];
    if(fatfs_getcwd(cwd_buf, sizeof(cwd_buf)) == FR_OK){
        cwd_buf[24] = '\0';
        printf("Current Working Directory: %s\n", cwd_buf);
    }
}









