#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "./libc/include/stdio.h"
#include "./libc/include/syscall.h"


const char *msg = "Hello from User Program 3.\n";
const char *file_name = "test.txt";

__attribute__((section(".text")))
void _start() {
    printf("%s", msg);

    if (syscall_fatfs_mount("", 0) == FR_OK) {
        printf("FAT filesystem mounted successfully.\n");
    } else {
        printf("Failed to mount FAT filesystem.\n");
    }

    if (syscall_fatfs_open(file_name, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        printf("File opened successfully: %s\n", file_name);
    }  else {
        printf("Failed to open file: %s\n", file_name);
    }

    while (true) {
        // Do nothing, just wait
    }
}