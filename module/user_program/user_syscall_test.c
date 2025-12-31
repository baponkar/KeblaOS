
/*
This user program will test system call from user space.

Build Date: 18.08.2025
Developed By: Bapon Kar
*/


// GCC included standard header files
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
// #include <stdarg.h>
// #include <limits.h>
// #include <float.h>
// #include <iso646.h>
// #include <stdatomic.h>
// #include <cpuid.h>

// My Standard headers
#include "../libc/include/assert.h"
#include "../libc/include/ctype.h"
#include "../libc/include/errno.h"
#include "../libc/include/limit.h"
#include "../libc/include/math.h"
#include "../libc/include/posix.h"
#include "../libc/include/stdio.h"
#include "../libc/include/stdlib.h"
#include "../libc/include/string.h"
#include "../libc/include/time.h"

#include "../libc/include/syscall.h"


void thread0_func(void *arg) {
    int *var = (int*) arg;
    while(true){
        printf("===> Thread 0 is Start...\n");
        sleep_seconds(1);// delay 100 milli seconds
        printf("===> Thread 0 is End...\n");
    }
}


void thread1_func(void* arg) {
    while(true) {
        printf("===> Thread 1 is Start...\n");
        sleep_seconds(1);    // delay 100 milli seconds
        printf("===> Thread 1 is End...\n");
    }
}

void halt(){
    while (true){
        // Do nothing
    }
}



void user_syscall_test(){                      // _start define in user_linker_x86.ld

    int disk_no = 1;   // Boot Disk is 0, User Disk is 1

    int64_t vfs_init_res = syscall_vfs_init(disk_no);
    if(vfs_init_res == -1){
        printf("VFS Initialization on Disk 1 failed!\n");
        return;
    }
    printf("VFS Initialization on Disk %d successful!\n", disk_no);

    uint64_t mount_res = syscall_mount(disk_no);
    if(mount_res == -1){
        printf("Mounting disk %d failed!\n", disk_no);
        return;
    }
    printf("Mounting %d successful!\n", disk_no);

    // void snprintf(char* buf, size_t size, const char* format, ...)
    char *file_path = "1:/TESTFILE.TXT";
    uint64_t flags = FA_CREATE_ALWAYS | FA_WRITE;
    void *file_node = (void *)syscall_open(disk_no, file_path, flags);
    if(file_node == NULL){
        printf("Opening or Creating file %s failed!\n", file_path);
        return;
    }
    printf("File %s opened or created successfully!\n\n", file_path);

    // ----------------------------------------------------------------------------------
    // char data[128] = "This string written by user system call.";

    char *data = (char *)syscall_uheap_alloc(128, ALLOCATE_DATA);
    memcpy(data, "This string written by user system call.", 40);

    // printf("Writing data %s(length %d) to %s file\n", data, (int)strlen(data), file_path);

    uint64_t write_res = syscall_write(disk_no, file_node, data, strlen(data));
    if(write_res == -1){
        printf("Writing %s file failed!\n", file_path);
        return;
    }
    printf("Writing to %s file successful! Wrote %d bytes.\n", file_path, write_res);

    uint64_t close_res = syscall_close(disk_no, file_node);
    if(close_res == -1){
        printf("The file %s close failed!\n", file_path);
        return;
    }
    printf("The file %s closed successfully!\n", file_path);

    // ----------------------------------------------------------------------------------
    file_node = (void *)syscall_open(1, file_path, FA_READ | FA_OPEN_EXISTING | FA_WRITE);
    if(file_node == NULL){
        printf("Opening file %s failed!\n", file_path);
        return;
    }
    printf("File %s opened successfully for reading!\n", file_path);
    

    char buf[128];
    uint64_t read_res = syscall_read(disk_no, file_node, 0, buf, 127);
    if(read_res < 0){
        printf("Reding file %s is failed!\n", file_path);
        return;
    }
    buf[127] = '\0';
    printf("Reading Successfull %s content: %s\n", file_path, buf);


    uint64_t lseek_res = syscall_lseek(disk_no, file_node, strlen(data));
    if(lseek_res != 0){
        printf("Lseek Failed!\n");
    }
    printf("Lseek Successful! File pointer moved to offset %d\n", strlen(data));


    char *new_data = (char *) syscall_uheap_alloc(128, ALLOCATE_DATA);
    memcpy(new_data, "This is new data appended after lseek.", 40);
    uint64_t write_res_1 = syscall_write(disk_no, file_node, new_data, strlen(new_data));
    if(write_res_1 == -1){
        printf("Writing %s file failed!\n", file_path);
        return;
    }
    printf("Writing to %s file successful! Wrote %d bytes.\n", file_path, write_res_1);


    lseek_res = syscall_lseek(disk_no, file_node, 0);
    if(lseek_res != 0){
        printf("Lseek Failed!\n");
    }
    printf("Lseek Successful! File pointer moved to offset %d\n", 0);

    char buf_1[128];
    uint64_t read_res_1 = syscall_read(disk_no, file_node, 0, buf_1, 127);
    if(read_res_1 < 0){
        printf("Reding file %s is failed!\n", file_path);
        return;
    }
    buf_1[127] = '\0';
    printf("Reading again successfull %s content: %s\n", file_path, buf_1);


    close_res = syscall_close(disk_no, (void *)file_node);
    if(close_res == -1){
        printf("The file %s close failed!\n", file_path);
        return;
    }
    printf("The file %s closed successfully!\n", file_path);


    uint64_t unlink_res = syscall_unlink(disk_no, file_path);
    if(unlink_res != 0){
        printf("file %s deleting failed! with error code %d\n", file_path, unlink_res);
        return;
    }
    printf("file %s deleted successfully!\n", file_path);

    uint64_t list_res = syscall_list_dir(disk_no, "1:/");
    if(list_res != 0){
        printf("Listing directory failed with error code %d\n", list_res);
        return;
    }
    printf("Listing directory successful!\n");

    // ------------------------------------------------------------------------
    // Process-Thread Tests
    char *process_name = "My_process";
    void *process = syscall_create_process(process_name);
    if(!process){
        printf("Creating \"%s\"s process failed!\n", process_name);
        return;
    }
    printf("Process \"%s\" created successfully!\n", process_name);

    const char *thread_name_1 = "thread_1";
    void *thread_1 = syscall_create_thread(process, thread_name_1, (void *) &thread0_func, NULL);
     if(!thread_1){
        printf("Creating \"%s\"s process failed!\n", thread_name_1);
        return;
    }
    

    printf("Creating Thred thread_2\n");
    const char *thread_name_2 = "thread_2";
    void *thread_2 = syscall_create_thread(process, thread_name_2, (void *) &thread1_func, NULL);
     if(!thread_2){
        printf("Creating \"%s\"s process failed!\n", thread_name_2);
        return;
    }

    sleep_seconds(2);
    printf("Sleep Time over!\n");

    halt();
}















