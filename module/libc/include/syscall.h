#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


enum int_syscall_number {

    // FatFs System Calls
    // File Access
    INT_SYSCALL_FATFS_OPEN      = 51,  // 0x33
    INT_SYSCALL_FATFS_CLOSE     = 52,  // 0x34
    INT_SYSCALL_FATFS_READ      = 53,  // 0x35
    INT_SYSCALL_FATFS_WRITE     = 54,  // 0x36
    INT_SYSCALL_FATFS_LSEEK     = 55,  // 0x37
    INT_SYSCALL_FATFS_TRUNCATE  = 56,  // 0x38
    INT_SYSCALL_FATFS_SYNC      = 57,  // 0x39
    INT_SYSCALL_FATFS_FORWARD   = 58,  // 0x3A
    INT_SYSCALL_FATFS_EXPAND    = 59,  // 0x3B
    INT_SYSCALL_FATFS_GETS      = 60,  // 0x3C
    INT_SYSCALL_FATFS_PUTC      = 61,  // 0x3D
    INT_SYSCALL_FATFS_PUTS      = 62,  // 0x3E
    INT_SYSCALL_FATFS_PRINTF    = 63,  // 0x3F
    INT_SYSCALL_FATFS_TELL      = 64,  // 0x40
    INT_SYSCALL_FATFS_EOF       = 65,  // 0x41
    INT_SYSCALL_FATFS_SIZE      = 66,  // 0x42
    INT_SYSCALL_FATFS_ERROR     = 67,  // 0x43

    // Directory Access
    INT_SYSCALL_FATFS_OPENDIR   = 68,  // 0x44
    INT_SYSCALL_FATFS_CLOSEDIR  = 69,  // 0x45
    INT_SYSCALL_FATFS_READDIR   = 70,  // 0x46
    INT_SYSCALL_FATFS_FINDFIRST = 71,  // 0x47
    INT_SYSCALL_FATFS_FINDNEXT  = 72,  // 0x48

    // File and Directory Management
    INT_SYSCALL_FATFS_STAT      = 73,  // 0x49
    INT_SYSCALL_FATFS_UNLINK    = 74,  // 0x4A
    INT_SYSCALL_FATFS_RENAME    = 75,  // 0x4B
    INT_SYSCALL_FATFS_CHMOD     = 76,  // 0x4C
    INT_SYSCALL_FATFS_UTIME     = 77,  // 0x4D
    INT_SYSCALL_FATFS_MKDIR     = 78,  // 0x4E
    INT_SYSCALL_FATFS_CHDIR     = 79,  // 0x4F
    INT_SYSCALL_FATFS_CHDRIVE   = 80,  // 0x50
    INT_SYSCALL_FATFS_GETCWD    = 81,  // 0x51

    // Volume Management and System Configuration
    INT_SYSCALL_FATFS_MOUNT     = 82,  // 0x52
    INT_SYSCALL_FATFS_MKFS      = 83,  // 0x53
    INT_SYSCALL_FATFS_FDISK     = 84,  // 0x54
    INT_SYSCALL_FATFS_GETFREE   = 85,  // 0x55
    INT_SYSCALL_FATFS_GETLABEL  = 86,  // 0x56
    INT_SYSCALL_FATFS_SETLABEL  = 87,  // 0x57
    INT_SYSCALL_FATFS_SETCP     = 88,  // 0x58

    // System Calls
    INT_SYSCALL_READ            = 89,  // 0x59
    INT_SYSCALL_PRINT           = 90,  // 0x5A
    INT_SYSCALL_EXIT            = 91,  // 0x5B
    INT_SYSCALL_PRINT_RAX       = 92   // 0x5C
};


typedef enum {
	FR_OK = 0,				/* (0) Function succeeded */
	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,				/* (2) Assertion failed */
	FR_NOT_READY,			/* (3) The physical drive does not work */
	FR_NO_FILE,				/* (4) Could not find the file */
	FR_NO_PATH,				/* (5) Could not find the path */
	FR_INVALID_NAME,		/* (6) The path name format is invalid */
	FR_DENIED,				/* (7) Access denied due to a prohibited access or directory full */
	FR_EXIST,				/* (8) Access denied due to a prohibited access */
	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,			/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,		/* (13) Could not find a valid FAT volume */
	FR_MKFS_ABORTED,		/* (14) The f_mkfs function aborted due to some problem */
	FR_TIMEOUT,				/* (15) Could not take control of the volume within defined period */
	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated or given buffer is insufficient in size */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > FF_FS_LOCK */
	FR_INVALID_PARAMETER	/* (19) Given parameter is invalid */
} FRESULT;

/* File access mode and open method flags (3rd argument of f_open function) */
#define	FA_READ				0x01
#define	FA_WRITE			0x02
#define	FA_OPEN_EXISTING	0x00
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define	FA_OPEN_APPEND		0x30

enum allocation_type {
    ALLOCATE_CODE = 0x1,   // Allocate for code
    ALLOCATE_DATA = 0x2,   // Allocate for data
    ALLOCATE_STACK = 0x3,  // Allocate for stack
};

int syscall_read(uint8_t *buffer, size_t size);
int syscall_print(const char *msg);
int syscall_exit();
int syscall_print_rax();

uint64_t syscall_uheap_alloc(size_t size, enum allocation_type type);
uint64_t uheap_free(void *ptr, size_t size);

uint64_t syscall_fatfs_open(const char *path, uint64_t mode);
uint64_t syscall_fatfs_close(void *file);
uint64_t syscall_fatfs_read(void *file, void *buf, uint32_t btr);
uint64_t syscall_fatfs_write(void *file, const void *buf, uint32_t btw);

uint64_t syscall_fatfs_mount(char *path, uint8_t opt);


