
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
void _star_1t() {

    printf("%s", msg);
    
    //----------------------------------File Manage ----------------------------------------//
    // Create a file
    uint64_t file = syscall_open(file_name, (uint64_t)FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    if (file == (uint64_t)-1) {
        printf("Failed to open file: %s\n", file_name);
        return;
    }else{
        printf("File opened successfully: %s, file ptr.: %x\n", file_name, file);
    }
    
    
    uint64_t write_res = syscall_write((void*)file, (void *)msg, strlen(msg));
    
    if (is_fs_error(write_res)) {
        printf("Write failed: %x\n", write_res);
        return;
    } else {
        printf("Successfully wrote %x bytes to %s\n", write_res, file_name);
    }
    
    // Reset file pointer to beginning
    uint64_t lseek_res = syscall_lseek((void *)file, 0);
    
    if (lseek_res != 0) {
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
        printf("Successfully read %x bytes from %s. Content: %s", read_res, file_name, buffer);
    }

    uint64_t close_res = syscall_close((void *)file);

    if(is_fs_error(close_res)){
        printf("Close Failed! : %x\n\n", close_res);
    } else {
        printf("Successfully close the file!\n\n");
    }

    // ---------------------------------------------------------------------------------------- //

    // Reopen the file to read and write again
    uint64_t file_1 = syscall_open(file_name, (uint64_t)FA_WRITE | FA_READ);
    if (file_1 == (uint64_t)-1) {
        printf("Failed to open file: %s\n", file_name);
        return;
    }else{
        printf("File opened successfully: %s, file ptr.: %x\n", file_name, file_1);
    }
    
    // Reset file pointer to beginning 
    uint64_t lseek_res_1 = syscall_lseek((void *)file_1, 0);
    
    if (lseek_res_1 != 0) {
        printf("Lseek failed.\n");
        return;
    }else{
        printf("Lseek success\n");
    }
    
    // Read the file again
    char buffer_1[128];
    uint64_t read_res_1 = syscall_read((void *)file_1, buffer_1, strlen(msg));
    
    if (is_fs_error(read_res_1)) {
        printf("Read failed: %x\n", read_res_1);
        return;
    } else {
        buffer_1[read_res_1] = '\0'; // Null-terminate the string
        printf("Successfully read %x bytes from %s.Contents: %s", read_res_1, file_name, buffer_1);
    }

    // Close the file again.
    uint64_t close_res_1 = syscall_close((void *)file_1);

    if(is_fs_error(close_res_1)){
        printf("Close Failed! : %x\n\n", close_res_1);
    } else {
        printf("Successfully close the file!\n\n");
    }

    // ----------------------------------------Directory Manage-----------------------------------//
    
    // Create a directory
    const char * dir_name = "/home";
    uint64_t mkdir = syscall_mkdir(dir_name);
    if(mkdir == 0){
        printf("Directory creation success\n");
    }else{
        printf("Directory creation failed\n");
    }

    // Open directory
    uint64_t dir = syscall_opendir(dir_name);
    if(dir > 0){
        printf("Directory open success\n");
    }else{
        printf("Directory open failed\n");
    }

    // Read Directory
    char * read_dir_name = (char *) syscall_readdir((void*) dir);
    if(read_dir_name != -1){
        printf("Directory reading success. Directory Name: %s\n", read_dir_name);
    }else{
        printf("Directory reading failed\n");
    }

    // Close directory
    uint64_t close_dir_res = syscall_closedir((void *)dir);
    if(close_dir_res == 0){
        printf("Directory successfully closed!\n");
    } else{
        printf("Directory Failed to closed!\n");
    }


    while (true) {} // Halt
}






