#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../include/time.h"


enum int_syscall_number {

     // Time Management
    INT_TIME = 1,
    INT_CLOCK_GETTIME = 2,
    INT_CLOCK_GETTIMEOFDAY = 3,
    INT_TIMES = 4,

    INT_SYSCALL_GET_TIME = 49,
    INT_SYSCALL_GET_UP_TIME = 50,

    // FatFs System Calls
    // File Access
    INT_SYSCALL_OPEN      = 51,  // 0x33
    INT_SYSCALL_CLOSE     = 52,  // 0x34
    INT_SYSCALL_READ      = 53,  // 0x35
    INT_SYSCALL_WRITE     = 54,  // 0x36
    INT_SYSCALL_LSEEK     = 55,  // 0x37
    INT_SYSCALL_TRUNCATE  = 56,  // 0x38
    INT_SYSCALL_SYNC      = 57,  // 0x39
    INT_SYSCALL_FORWARD   = 58,  // 0x3A
    INT_SYSCALL_EXPAND    = 59,  // 0x3B
    INT_SYSCALL_GETS      = 60,  // 0x3C
    INT_SYSCALL_PUTC      = 61,  // 0x3D
    INT_SYSCALL_PUTS      = 62,  // 0x3E
    INT_SYSCALL_PRINTF    = 63,  // 0x3F
    INT_SYSCALL_TELL      = 64,  // 0x40
    INT_SYSCALL_EOF       = 65,  // 0x41
    INT_SYSCALL_SIZE      = 66,  // 0x42
    INT_SYSCALL_ERROR     = 67,  // 0x43

    // Directory Access
    INT_SYSCALL_OPENDIR   = 68,  // 0x44
    INT_SYSCALL_CLOSEDIR  = 69,  // 0x45
    INT_SYSCALL_READDIR   = 70,  // 0x46
    INT_SYSCALL_FINDFIRST = 71,  // 0x47
    INT_SYSCALL_FINDNEXT  = 72,  // 0x48

    // File and Directory Management
    INT_SYSCALL_STAT      = 73,  // 0x49
    INT_SYSCALL_UNLINK    = 74,  // 0x4A
    INT_SYSCALL_RENAME    = 75,  // 0x4B
    INT_SYSCALL_CHMOD     = 76,  // 0x4C
    INT_SYSCALL_UTIME     = 77,  // 0x4D
    INT_SYSCALL_MKDIR     = 78,  // 0x4E
    INT_SYSCALL_CHDIR     = 79,  // 0x4F
    INT_SYSCALL_CHDRIVE   = 80,  // 0x50
    INT_SYSCALL_GETCWD    = 81,  // 0x51

    // Volume Management and System Configuration
    INT_SYSCALL_MOUNT     = 82,  // 0x52
    INT_SYSCALL_MKFS      = 83,  // 0x53
    INT_SYSCALL_FDISK     = 84,  // 0x54
    INT_SYSCALL_GETFREE   = 85,  // 0x55
    INT_SYSCALL_GETLABEL  = 86,  // 0x56
    INT_SYSCALL_SETLABEL  = 87,  // 0x57
    INT_SYSCALL_SETCP     = 88,  // 0x58

    // System Calls
    INT_SYSCALL_KEYBOARD_READ   = 89,  // 0x59
    INT_SYSCALL_PRINT           = 90,  // 0x5A
    INT_SYSCALL_EXIT            = 91,  // 0x5B
    INT_SYSCALL_PRINT_RAX       = 92,  // 0x5C

    // User Memory Allocation
    INT_SYSCALL_ALLOC           = 93,  // 0x5D
    INT_SYSCALL_FREE            = 94,   // 0x5E

    // Process Management
    INT_CREATE_PROCESS          = 95,   // 0x5F
    INT_DELETE_PROCESS          = 96,   // 0x60
    INT_GET_PROCESS_FROM_PID    = 97,   // 0x61
    INT_GET_CURRENT_PROCESS     = 98,   // 0x62

    // Thread Management
    INT_CREATE_THREAD           = 99,   // 0x63
    INT_DELETE_THREAD           = 100,  // 0x64

    INT_SYSCALL_LIST            = 101,  // 0x65
    INT_VFS_INIT                = 102,  // 0x66
    INT_VFS_MKFS                = 103,  // 0x67

        // VGA 
    INT_VGA_SETPIXEL            = 104,  // 0x68
    INT_VGA_GETPIXEL            = 105,  // 0x69
    INT_VGA_CLEAR               = 106,  // 0x6a
    INT_VGA_DISPLAY_IMAGE       = 107,  // 0x6b
    INT_VGA_DISPLAY_TRANSPARENT_IMAGE = 108, // 0x6c

    // ACPI
    INT_ACPI_POWEROFF           = 110,
    INT_ACPI_REBOOT             = 111,

    // Serial Print
    INT_SYSCALL_SERIAL_PRINT    = 112

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

int syscall_keyboard_read(uint8_t *buffer, size_t size);
int syscall_print(const char *msg);
int syscall_exit();
int syscall_print_rax();

uint64_t syscall_uheap_alloc(size_t size, enum allocation_type type);
uint64_t syscall_uheap_free(void *ptr, size_t size);


// FatFs File Manage
int64_t syscall_vfs_mkfs(int fs_type, char *disk);
int64_t syscall_vfs_init(char *fs_name);
uint64_t syscall_mount(char *path);
uint64_t syscall_open(const char *path, uint64_t flags);
uint64_t syscall_close(void *file);
uint64_t syscall_read(void *file, uint64_t offset, void *buf, uint32_t size);
uint64_t syscall_write(void *file, uint64_t offset, void *buf, uint32_t btw);


uint64_t syscall_lseek(void *file, uint32_t offs);
uint64_t syscall_unlink(char *path);


// FatFs Directory Manage
uint64_t syscall_opendir(const char *path);
uint64_t syscall_closedir(void * dir_ptr);
uint64_t syscall_readdir(void * dir_ptr);
uint64_t syscall_mkdir(void * dir_ptr);
int syscall_list_dir(const char* path);

int syscall_getcwd(void *buf, size_t size);
int syscall_chdir(const char *path);
int syscall_chdrive(const char *path);


// Process Manage
void *syscall_create_process(char* process_name);
int syscall_delete_process(void *process);
void *syscall_get_process_from_pid(size_t pid);
void *syscall_get_current_process();

// Thread Manage
void *syscall_create_thread(void* parent, const char* thread_name, void (*function)(void*), void* arg);
void *syscall_delete_thread(void *thread);



// Time Manage
time_t syscall_time(time_t *t);
int syscall_clock_gettime(int clk_id, struct timespec *tp);
int syscall_gettimeofday(struct timeval *tv, struct timezone *tz);
clock_t syscall_times(struct tms *buf);
uint64_t syscall_get_uptime(void);




// VGA 
int syscall_set_pixel(int x, int y, uint32_t color);
uint32_t syscall_get_pixel(int x, int y);
int syscall_cls_color(uint32_t color);
int syscall_display_image( int x, int y, const uint64_t* image_data, int img_width, int img_height);
int syscall_display_transparent_image( int x, int y, const uint64_t* image_data, int img_width, int img_height);




