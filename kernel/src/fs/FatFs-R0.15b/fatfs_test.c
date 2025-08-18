
#include "../../lib/string.h"
#include "../../lib/stdio.h"        // printf

#include "../../memory/kheap.h"
#include "../../memory/vmm.h"

#include "fatfs_test.h"


extern FATFS *fatfs;

void fatfs_list_dir(const char *path) {

    DIR dir;
    FILINFO fno;

    FRESULT res = f_opendir(&dir, path);
    if (res != FR_OK) {
        printf("[FATFS] Failed to open dir: %s, error: %d\n", path, res);
        return;
    }

    // printf("[FATFS] Listing directory: %s: ", path);

    while (1) {
        memset(&fno, 0, sizeof(FILINFO)); 
        res = f_readdir(&dir, &fno);

        if (res != FR_OK) {
            printf("[FATFS] Read error: %d\n", res);
            break;
        }

        if (fno.fname[0] == 0) break;

        // Dump first few raw bytes
        printf((fno.fattrib & AM_DIR) ? "%s " : "%s(%d bytes) " ,fno.fname, (uint64_t)fno.fsize);
    }

    printf("\n");

    f_closedir(&dir);
}



char *get_parent_dir(const char *path) {
    // Find the last '/' in the path
    const char *last_slash = strrchr(path, '/');
    if (last_slash == NULL || last_slash == path) {
        return NULL; // No parent directory
    }

    // Calculate the length of the parent directory path
    size_t parent_length = last_slash - path;

    // Allocate memory for the parent directory path
    char parent_path[parent_length + 1];
    if (parent_path == NULL) {
        return NULL; // Memory allocation failed
    }

    // Copy the parent directory path
    strncpy(parent_path, path, parent_length);
    parent_path[parent_length] = '\0'; // Null-terminate the string

    return parent_path;
}




void test_fatfs() {

    printf("\n[FATFS] Starting FATFS test...\n");

    FIL file;
    char buffer[128];
    UINT br, bw;

    // Create and write to the file
    if (f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        const char* msg = "Hello from KeblaOS!";
        FRESULT res = f_write(&file, msg, strlen(msg), &bw);
        if (res == FR_OK) {
            printf("[FATFS] Successfully wrote %x bytes to test.txt\n", bw);
        } else {
            printf("[FATFS] Write failed: %d\n", res);
        }
        f_close(&file);
    } else {
        printf("[FATFS] Failed to create file\n");
    }

    // Reopen the file for reading
    if (f_open(&file, "test.txt", FA_READ) == FR_OK) {
        FRESULT res = f_read(&file, buffer, sizeof(buffer) - 1, &br);
        if (res == FR_OK) {
            buffer[br] = 0;
            printf("[FATFS] Read from file: %s\n", buffer);
        } else {
            printf("[FATFS] Read failed: %d\n", res);
        }
        f_close(&file);
    } else {
        printf("[FATFS] Failed to open file for reading\n");
    }


    // Get Current Volume label
    TCHAR label[FF_LFN_BUF + 1];
    DWORD vsn;
    if (f_getlabel("", label, &vsn) == FR_OK) {
        printf("[FATFS] Volume label: %s, VSN: %d\n", label, vsn);
    } else {
        printf("[FATFS] Failed to get volume label\n");
    }

    // Set Current Volume Label
    if (f_setlabel("KeblaOS") == FR_OK) {
        printf("[FATFS] Successfully set volume label to KeblaOS\n");
    } else {
        printf("[FATFS] Failed to set volume label\n");
    }

    // Get Current Volume label
    TCHAR new_label[FF_LFN_BUF + 1];
    DWORD new_vsn;
    if (f_getlabel("", new_label, &vsn) == FR_OK) {
        printf("[FATFS] After set label Volume label: %s, VSN: %d\n", (char *)new_label, new_vsn);
    } else {
        printf("[FATFS] Failed to get volume label\n");
    }

    // Create a directory
    if (f_mkdir("home") == FR_OK) {
        printf("[FATFS] Successfully created directory home\n");
    } else {
        printf("[FATFS] Failed to create directory\n");
    }

    // List the root directory
    fatfs_list_dir("/");

    // Get Current Directory
    TCHAR cwd[FF_MAX_LFN];
    if (f_getcwd(cwd, FF_MAX_LFN) == FR_OK) {
        printf("[FATFS] Current directory: %s\n", cwd);
    } else {
        printf("[FATFS] Failed to get current directory\n");
    }

    // Change current directory
    if (f_chdir("/home") == FR_OK) {
        printf("[FATFS] Successfully changed to directory test_dir\n");
    } else {
        printf("[FATFS] Failed to change directory\n");
    }

    // Get current directory after change
    TCHAR new_cwd[FF_MAX_LFN];
    if (f_getcwd(new_cwd, 255) == FR_OK) {
        printf("[FATFS] Current directory: %s\n", new_cwd);
    } else {
        printf("[FATFS] Failed to get current directory\n");
    }

    // Creating a new file in the new directory
    if (f_open(&file, "new_file.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        const char* msg = "Hello from KeblaOS in new directory!";
        FRESULT res = f_write(&file, msg, strlen(msg), &bw);
        if (res == FR_OK) {
            printf("[FATFS] Successfully wrote %x bytes to new_file.txt\n", bw);
        } else {
            printf("[FATFS] Write failed: %d\n", res);
        }
        f_close(&file);
    } else {
        printf("[FATFS] Failed to create file in new directory\n");
    }

    // List the new directory
    fatfs_list_dir("/home");

    // Reading the file from the new directory
    if (f_open(&file, "new_file.txt", FA_READ) == FR_OK) {
        memset(buffer, 0, sizeof(buffer)); // Clear buffer
        UINT br;
        FRESULT res = f_read(&file, buffer, sizeof(buffer) - 1, &br);
        if (res == FR_OK) {
            buffer[br] = 0;
            printf("[FATFS] Read from new_file.txt: %s\n", buffer);
        } else {
            printf("[FATFS] Read failed: %d\n", res);
        }
        f_close(&file);
    } else {
        printf("[FATFS] Failed to open new_file.txt for reading\n");
    }

    char *par = get_parent_dir("/home/new_file.txt");
    printf("[FATFS] Parent directory of /home/new_file.txt: %s\n", par != NULL ? par : "NULL");

    printf("[FATFS] FATFS test completed.\n\n");
}





