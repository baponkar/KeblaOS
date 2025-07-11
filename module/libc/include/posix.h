// #pragma once

// #include <stdint.h>
// #include <stdbool.h>
// #include <stddef.h>

// // Standard POSIX types
// typedef uint16_t mode_t;
// typedef int32_t pid_t;
// typedef int64_t off_t;
// typedef uint32_t useconds_t;
// typedef intptr_t ssize_t;
// typedef uint64_t dev_t;
// typedef uint64_t ino_t;
// typedef uint16_t nlink_t;
// typedef uint32_t uid_t;
// typedef uint32_t gid_t;
// typedef int64_t blksize_t;
// typedef int64_t blkcnt_t;


// #define S_IRUSR  0x0100  // Owner read
// #define S_IWUSR  0x0080  // Owner write
// #define S_IXUSR  0x0040  // Owner execute

// #define S_IRGRP  0x0020  // Group read
// #define S_IWGRP  0x0010  // Group write
// #define S_IXGRP  0x0008  // Group execute

// #define S_IROTH  0x0004  // Others read
// #define S_IWOTH  0x0002  // Others write
// #define S_IXOTH  0x0001  // Others execute


// typedef enum {
//     O_RDONLY = 0x0000,     // Open for reading only
//     O_WRONLY = 0x0001,     // Open for writing only
//     O_RDWR   = 0x0002,     // Open for reading and writing
//     O_ACCMODE = 0x0003,    // Mask for access mode

//     O_CREAT  = 0x0100,     // Create file if it does not exist
//     O_EXCL   = 0x0200,     // Error if O_CREAT and file exists
//     O_TRUNC  = 0x0400,     // Truncate file to zero length
//     O_APPEND = 0x0800,     // Set append mode
// } flags_t;


// struct stat {
//     dev_t     st_dev;     // Device ID
//     ino_t     st_ino;     // Inode number
//     mode_t    st_mode;    // File mode (type + permissions)
//     nlink_t   st_nlink;   // Number of hard links
//     uid_t     st_uid;     // User ID of owner
//     gid_t     st_gid;     // Group ID of owner
//     dev_t     st_rdev;    // Device ID (if special file)
//     off_t     st_size;    // Total size, in bytes
//     blksize_t st_blksize; // Block size for filesystem I/O
//     blkcnt_t  st_blocks;  // Number of 512B blocks allocated
//     struct timeval {
//         int64_t tv_sec;   // Seconds
//         int64_t tv_usec;  // Microseconds
//     } st_atim, st_mtim, st_ctim; // Access, mod, and status change times
// };

// typedef struct DIR {
//     int fd;                  // File descriptor of the directory
//     int index;               // Current read index
//     struct dirent *entry;    // Current entry (cached)
// } DIR;

// struct dirent {
//     ino_t d_ino;             // Inode number
//     char d_name[256];        // Filename (null-terminated)
//     uint8_t d_type;          // Type (DT_REG, DT_DIR, etc.)
// };

// // File type macros
// #define DT_UNKNOWN  0
// #define DT_REG      1
// #define DT_DIR      2

// struct timeval {
//     int64_t tv_sec;   // seconds
//     int64_t tv_usec;  // microseconds
// };

// struct timezone {
//     int tz_minuteswest; // minutes west of Greenwich
//     int tz_dsttime;     // type of DST correction
// };

// #define SEEK_SET 0  // Set file offset to offset
// #define SEEK_CUR 1  // Set file offset to current plus offset
// #define SEEK_END 2  // Set file offset to EOF plus offset


// #define PROT_READ   0x1     // Page can be read
// #define PROT_WRITE  0x2     // Page can be written
// #define PROT_EXEC   0x4     // Page can be executed
// #define PROT_NONE   0x0     // Page cannot be accessed

// #define MAP_SHARED    0x01  // Share changes
// #define MAP_PRIVATE   0x02  // Changes are private
// #define MAP_ANONYMOUS 0x20  // Don't use a file

// #define TCGETS 0x5401
// #define TCSETS 0x5402
// // Add custom ioctl commands as needed

// struct utsname {
//     char sysname[128];    // Operating system name
//     char nodename[128];   // Name within "some implementation-defined network"
//     char release[128];    // OS release
//     char version[128];    // OS version
//     char machine[128];    // Hardware identifier
// };



// // File Operations
// int open(const char *pathname, int flags, mode_t mode);
// int read(int fd, void *buf, size_t count);
// int write(int fd, const void *buf, size_t count);
// int close(int fd);
// int lseek(int fd, off_t offset, int whence);
// int ftruncate(int fd, off_t length);
// int unlink(const char *pathname);
// int stat(const char *pathname, struct stat *buf);


// // Directory Operations
// int opendir(const char *name);
// int readdir(DIR *dirp);
// int closedir(DIR *dirp);
// int mkdir(const char *pathname, mode_t mode);
// int rmdir(const char *pathname);


// // Process Management
// int fork(); // Creates a new process by duplicating the current process.
// int execve(const char *path, char *const argv[], char *const envp[]); // Replaces the current process image with a new one.
// int exit(int status);   // Terminates the process with given status.
// int wait(int *status);
// int kill(pid_t pid, int sig);
// int getpid();
// int getppid();


// // Memory Management
// int sbrk(intptr_t increment);   // Increases or decreases the data segment (heap).
// int mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset); // Maps file or anonymous memory to process space.
// int munmap(void *addr, size_t length); // Unmaps memory allocated via mmap.


// // Console I/O
// int read();
// int write();


// // Time Management
// int sleep(unsigned int seconds);
// int usleep(useconds_t usec);    // Sleeps in microseconds.
// int gettimeofday(struct timeval *tv, struct timezone *tz);


// int chdir();    // Change Directory.
// int getcwd();   // Get Current Directory
// int dup();
// int dup2();
// int pipe();     // Inter Process Communication
// int selecct();
// int ioctl();	// Device control and configuration

