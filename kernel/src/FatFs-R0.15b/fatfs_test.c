
#include "fatfs_test.h"


FIL *create_file(const char* fname){
    FIL *fil = (FIL *) kheap_alloc(sizeof(FIL), ALLOCATE_DATA);
    FRESULT res;
    UINT bw;

    // Creating a file in the root directory
    res = f_open(fil, fname, FA_WRITE | FA_CREATE_ALWAYS);

    if(res != FR_OK) {
        kheap_free(fil, sizeof(FIL));
        return NULL; // Failed to create file
    }

    return fil; // Return the file pointer
}

int write_file(FIL *fil, const char* data) {

    FRESULT res;
    UINT bw;

    // Write data to the file
    res = f_write(fil, data, strlen(data), &bw);
    f_close(fil);
    kheap_free(fil, sizeof(FIL));

    return (res == FR_OK && bw == strlen(data)) ? 0 : -1; // Return 0 on success
}

int read_file(const char* fname, char* buffer, size_t buffer_size) {
    FIL fil;
    FRESULT res;
    UINT br;

    // Open the file for reading
    res = f_open(&fil, fname, FA_READ);
    if (res != FR_OK) {
        return -1; // Failed to open file
    }

    // Read data from the file
    res = f_read(&fil, buffer, buffer_size - 1, &br);
    if (res != FR_OK) {
        f_close(&fil);
        return -1; // Read failed
    }

    buffer[br] = '\0'; // Null-terminate the string
    f_close(&fil);
    return br; // Return number of bytes read
}



void test_fatfs() {
    FATFS fs;
    FIL file;
    char buffer[128];
    UINT br, bw;

    // Mount the filesystem
    FRESULT res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Mount failed: %d\n", res);
        return;
    }

    // Create and write to the file
    if (f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        const char* msg = "Hello from KeblaOS!";
        res = f_write(&file, msg, strlen(msg), &bw);
        if (res == FR_OK) {
            printf("Successfully wrote %x bytes to test.txt\n", bw);
        } else {
            printf("Write failed: %d\n", res);
        }
        f_close(&file);
    } else {
        printf("Failed to create file\n");
    }

    // Reopen the file for reading
    if (f_open(&file, "test.txt", FA_READ) == FR_OK) {
        res = f_read(&file, buffer, sizeof(buffer) - 1, &br);
        if (res == FR_OK) {
            buffer[br] = 0;
            printf("Read from file: %s\n", buffer);
        } else {
            printf("Read failed: %d\n", res);
        }
        f_close(&file);
    } else {
        printf("Failed to open file for reading\n");
    }
}