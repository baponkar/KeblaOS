
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "fat32.h"
#include "../ahci/ahci.h"


#include "fs.h"


void test_file_operations(HBA_MEM_T* abar) {

    // Create a file and write data to it
    const char* filename = "testfile.txt";
    const char* content = "Hello, this is a test file!\n";

    if (fat32_create_file(filename)) {
        printf("File '%s' created successfully.\n", filename);
    } else {
        printf("Failed to create file '%s'.\n", filename);
        return;
    }

    if (fat32_write_file(filename, (const uint8_t*)content, strlen(content))) {
        printf("Data written to '%s' successfully.\n", filename);
    } else {
        printf("Failed to write to '%s'.\n", filename);
        return;
    }

    // Read the file back
    uint8_t buffer[1024]; // Ensure the buffer is large enough for the file's content
    uint32_t max_size = sizeof(buffer);

    if (fat32_read_file(filename, buffer, max_size)) {
        printf("Data read from '%s':\n", filename);
        printf("%s\n", buffer);  // This prints the content of the file
    } else {
        printf("Failed to read file '%s'.\n", filename);
    }
}


void test_directory_operations(HBA_MEM_T* abar) {

    // Create a new directory
    const char* dir_name = "newdir";
    if (fat32_create_directory(dir_name)) {
        printf("Directory '%s' created successfully.\n", dir_name);
    } else {
        printf("Failed to create directory '%s'.\n", dir_name);
        return;
    }

    // Now create a file inside the new directory
    const char* filename = "newdir/testfile.txt";  // File path inside the new directory
    const char* content = "Hello, this is a test file inside a directory!\n";

    if (fat32_create_file(filename)) {
        printf("File '%s' created successfully.\n", filename);
    } else {
        printf("Failed to create file '%s'.\n", filename);
        return;
    }

    // Write data to the file inside the directory
    if (fat32_write_file(filename, (const uint8_t*)content, strlen(content))) {
        printf("Data written to '%s' successfully.\n", filename);
    } else {
        printf("Failed to write to '%s'.\n", filename);
        return;
    }

    // Read the file back
    uint8_t buffer[1024];  // Ensure the buffer is large enough for the file's content
    uint32_t max_size = sizeof(buffer);

    if (fat32_read_file(filename, buffer, max_size)) {
        printf("Data read from '%s':\n", filename);
        printf("%s\n", buffer);  // This prints the content of the file
    } else {
        printf("Failed to read file '%s'.\n", filename);
    }
}






