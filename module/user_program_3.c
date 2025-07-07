
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "./libc/include/stdio.h"
#include "./libc/include/string.h"
#include "./libc/include/syscall.h"


const char *msg = "Hello from User Program 3.\n";
const char *file_name = "test.txt";


// Helper function to check filesystem errors
static inline int is_fs_error(uint64_t res) {
    return (res == (uint64_t)-1) || (res == (uint64_t)0xFFFFFFFF);
}


__attribute__((section(".text")))
void _start() {

    printf("%s", msg);
    
    // Create a file
    uint64_t file = syscall_open(file_name, (uint64_t)FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    if (file == (uint64_t)-1) {
        printf("Failed to open file: %s\n", file_name);
        return;
    }else{
        printf("File opened successfully: %s, file ptr.: %x\n", file_name, file);
    }
    
    
    // ------Problems----
    uint64_t write_res = syscall_write((void*)file, (void *)msg, strlen(msg));
    
    if (is_fs_error(write_res)) {
        printf("Write failed: %x\n", write_res);
        return;
    } else {
        printf("Successfully wrote %x bytes to %s\n", write_res, file_name);
    }
    
    // Reset file pointer to beginning
    if (syscall_lseek((void *)file, 0) != 0) {
        printf("Lseek failed.\n");
        return;
    }else{
        printf("Lseek success\n");
    }
    
    // Read the file
    char buffer[128];
    uint64_t read_res = syscall_read((void *)file, buffer, strlen(msg));
    
    if (is_fs_error(read_res)) {
        printf("Read failed: %x\n", read_res);
        return;
    } else {
        buffer[read_res] = '\0'; // Null-terminate the string
        printf("Successfully read %x bytes: %s\n", read_res, buffer);
    }

    while (true) {} // Halt
}






