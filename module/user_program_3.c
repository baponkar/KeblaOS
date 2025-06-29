#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "./libc/include/stdio.h"
#include "./libc/include/syscall.h"


const char *msg = "Hello from User Program 3.\n";
const char *file_name = "test.txt";

__attribute__((section(".text")))
void _start() {
    // Print the message using the printf function from libc
    printf("%s", msg);

    uint64_t file_handle = syscall_fatfs_open(file_name, FA_READ | FA_WRITE | FA_CREATE_ALWAYS); // Open file in read/write mode
    printf("Test\n");

    if (file_handle == (uint64_t)-1) {
        printf("Failed to open file: %s\n", file_name);
    } else {
        printf("File opened successfully: %s\n", file_name);
        
        // Write some data to the file
        const char *data = "This is a test data written to the file.\n";
        int write_result = syscall_fatfs_write((void *)file_handle, data, sizeof(data));
        if (write_result < 0) {
            printf("Failed to write to file.\n");
        } else {
            printf("Data written successfully to the file.\n");
        }

        // Close the file
        syscall_fatfs_close((void *)file_handle);
    }

    // Infinite loop to keep the program running
    while (true) {
        // Do nothing, just wait
    }
}