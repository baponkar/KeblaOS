
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "libc/include/syscall.h"
#include "libc/include/stdio.h"
#include "libc/include/string.h"

#define MAX_INPUT 128

static inline int is_fs_error(uint64_t res) {
    return (res == (uint64_t)-1) || (res == (uint64_t)0xFFFFFFFF);
}

__attribute__((section(".text")))
void _start_12() {

    char input[MAX_INPUT];

    while (1) {
        printf("input> ");
        memset(input, 0, sizeof(input));
        syscall_keyboard_read((uint8_t*)input, MAX_INPUT);
        printf("\nYou typed: %s", input);

        char buffer[128];
        uint64_t read_bytes = 0;

        // Try to create a new file 
        void *file_node = (void *) syscall_open(input, FA_OPEN_EXISTING | FA_READ | FA_WRITE);

        // If the file already present
        if(!is_fs_error((uint64_t)file_node)){
            printf("%s file already present.\n", input);

            uint64_t lseek_res = syscall_lseek(file_node, 0);
            if(is_fs_error(lseek_res)){
                printf("Lseek is failed!\n");
            }else{
                printf("Lseek is success\n");
            }

            printf("Reading the file\n");
            read_bytes = syscall_read(file_node, buffer, sizeof(buffer) - 1);
            if ((int64_t)read_bytes < 0) {
                printf("Reading the file %s is failed!\n", input);
            } else if (read_bytes == 0) {
                printf("File is empty or at EOF: %s\n", input);
            } else {
                buffer[read_bytes] = '\0';
                printf("Success reading file name: %s, content: %s, read_bytes: %d\n", input, buffer, read_bytes);
            }
        } else { // Create a new file by giving name
            printf("Creating a new file: %s\n", input);
            file_node = (void *) syscall_open(input, FA_CREATE_ALWAYS | FA_READ | FA_WRITE);

            printf("Writing in the file\n");
            char *str = "Hello from user_program.c";
            int res = syscall_write(file_node, str, strlen(str));
            if(is_fs_error(res)){
                printf("Writing failed\n");
            }

            uint64_t lseek_res = syscall_lseek(file_node, 0);
            if(is_fs_error(lseek_res)){
                printf("Lseek is failed!\n");
            }else{
                printf("Lseek is success\n");
            }

            printf("Reading the file\n");
            read_bytes = syscall_read(file_node, buffer, sizeof(buffer) - 1);
            if ((int64_t)read_bytes < 0) {
                printf("Reading the file %s is failed!\n", input);
            } else if (read_bytes == 0) {
                printf("File is empty or at EOF: %s\n", input);
            } else {
                buffer[read_bytes] = '\0';
                printf("Success reading file name: %s, content: %s, read_bytes: %d\n", input, buffer, read_bytes);
            }
        }

        uint64_t close_res = syscall_close(file_node);
        printf("close_res: %x\n", close_res);
        if(close_res == 0){
            printf("Successfully %s closed\n", input);
        }else{
            printf("Failed to close %s\n", input);
        }

        uint64_t unlink_res = syscall_unlink(input);
        printf("unlink_res: %d\n", unlink_res);
        if(unlink_res == 0){
            printf("Successfully %s deleted\n", input);
        }else{
            printf("Failed to delete %s\n", input);
        }
        
    }

    while (true) {} // Halt
}







