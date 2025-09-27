
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

    printf("\nHello from users\'s program \'syscall_test.c\'\n");

    // Timer Test
    // test_time_functions();

    printf("Listing root directory \"/\"\n");
    char *root_dir_path = "/";
    if(syscall_list_dir(root_dir_path) != 0){
        printf("Listing %s directory failed!\n", root_dir_path);
    }

    printf("Creating file user_file.txt\n");
    char *file_path = "/user_file.txt";
    uint64_t flags = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;
    void *file_node = (void *)syscall_open(file_path, flags);
    if(file_node == NULL){
        printf("Opening or Creating file %s failed!\n", file_path);
        return;
    }

    char *data = "This string written by user system call.";
    printf("Writing %s file with \"%s\"\n", file_path, data);
    uint64_t write_res = syscall_write(file_node, 0, (void *)data, strlen(data));
    if(write_res == 0){
        printf("Writing %s file failed!\n", file_path);
        return;
    }
    
    printf("Reading %s file\n", file_path);
    char buf[128];
    uint64_t read_res = syscall_read(file_node, 0, (void *)buf, 128);
    if(read_res < 0){
        printf("Reding file %s is failed!\n", file_path);
        return;
    }
    printf("%s content: %s\n", file_path, buf);


    printf("Changing File Pointer(LSEEK)\n");
    uint64_t lseek_res = syscall_lseek(file_node, strlen(data));
    if(lseek_res != 0){
        printf("Lseek Failed!\n");
    }

    printf("Writing again after changing file pointer\n");
    char *new_data = "This is a new string.";
    printf("Writing %s file with \"%s\"\n", file_path, new_data);
    uint64_t write_res_1 = syscall_write(file_node, 0, (void *)new_data, (strlen(new_data) + strlen(data)));
    if(write_res_1 == 0){
        printf("Writing %s file failed!\n", file_path);
        return;
    }

    printf("Reading %s file again\n", file_path);
    char buf_1[128];
    uint64_t read_res_1 = syscall_read(file_node, 0, (void *)buf_1, 128);
    if(read_res_1 < 0){
        printf("Reding file %s is failed!\n", file_path);
        return;
    }
    printf("%s content: %s\n", file_path, buf);

    printf("Closing the %s file\n", file_path);
    uint64_t close_res = syscall_close((void *)file_node);
    if(close_res != 0){
        printf("The file %s close failed!\n", file_path);
        return;
    }

    printf("Deleting %s file\n", file_path);
    uint64_t unlink_res = syscall_unlink(file_path);
    if(unlink_res != 0){
        printf("file %s deeting failed!\n", file_path);
        return;
    }

    // Process-Thread Tests
    printf("Creating Process My_process\n");
    char *process_name = "My_process";
    void *process = syscall_create_process(process_name);
    if(!process){
        printf("Creating \"%s\"s process failed!\n", process_name);
        return;
    }

    printf("Creating Thred thread_1\n");
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









