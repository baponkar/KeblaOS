#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>




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



void int_syscall_init();

int syscall_read(uint8_t *buffer, size_t size);
int syscall_print(const char *msg);
int syscall_exit();

void syscall_test();



