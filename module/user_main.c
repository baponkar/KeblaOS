#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "libc/include/syscall.h"
#include "libc/include/stdio.h"
#include "libc/include/string.h"



__attribute__((section(".text")))
void _start(){
    printf("Welcome! This is User Program \"user_main.c\"\n");

    // initializing VFS
    if(syscall_vfs_init("fat") != 0){
        printf("VFS initialization is failed!\n");
    }
    
    // Format The First Disk
    char *disk = "0:";
    if(syscall_vfs_mkfs(2, disk)){
        printf("Formatting first disk failed!\n");
    }

    // Mount First Disk 0
    if(syscall_mount(disk) != 0){
        printf("Mount Device 0: is failed!\n");
    }

    // Current Working Directory
    char cwd_buf[512];
    if(syscall_getcwd(cwd_buf, sizeof(cwd_buf)) == 0){
        printf("cwd: %s\n", cwd_buf);
    }

    // Listing root directory
    printf("Listing root directory: \'/\'\n");
    if(syscall_list_dir("/") != 0){
        printf("Listing directory / failed!\n");
    }

    // Open a File to read
    char *file_path = "/TESTFILE.TXT";
    void *file = (void*)syscall_open(file_path, FA_READ);
    if(!file){
        printf("Opening file %x failed\n", (uint64_t)file);
    }

    // Reading /TESTFILE.txt
    char buf[512];
    if(syscall_read(file, 0, buf, sizeof(buf) - 1) == -1){
        printf("File %s read failed!\n", file_path);
    }else{
        buf[sizeof(buf)] = '\0';
        printf("%s: %s", file_path, buf);
    }

    while (true) {}     // Halt
}


