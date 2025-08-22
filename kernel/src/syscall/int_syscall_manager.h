
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


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
    INT_SYSCALL_OPEN      = 51,  // 0x33 : Open a file
    INT_SYSCALL_CLOSE     = 52,  // 0x34 : Close a file
    INT_SYSCALL_READ      = 53,  // 0x35 : Read from a file
    INT_SYSCALL_WRITE     = 54,  // 0x36 : Write to a file
    INT_SYSCALL_LSEEK     = 55,  // 0x37 : Set file pointer position (lseek)
    INT_SYSCALL_TRUNCATE  = 56,  // 0x38 : Truncate a file i.e.
    INT_SYSCALL_SYNC      = 57,  // 0x39 : Synchronize a file with storage
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
    INT_SYSCALL_FREE            = 94,  // 0x5E

    // Process Management
    INT_CREATE_PROCESS          = 95,   // 0x5F
    INT_DELETE_PROCESS          = 96,   // 0x60
    INT_GET_PROCESS_FROM_PID    = 97,   // 0x61
    INT_GET_CURRENT_PROCESS     = 98,   // 0x62

    // Thread Management
    INT_CREATE_THREAD           = 99,   // 0x63
    INT_DELETE_THREAD           = 100,  // 0x64

    INT_SYSCALL_LIST            = 101
};

void int_syscall_init();

// rax store system call number
// arguments in rdi, rsi, rdx, r10, r8, r9 
static uint64_t system_call(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10, uint64_t r8, uint64_t r9);

void int_syscall_test();




