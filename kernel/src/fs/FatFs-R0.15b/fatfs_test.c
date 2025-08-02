


#include "fatfs_test.h"


void fatfs_list_dir(const char *path) {

    DIR dir;
    FILINFO fno;

    FRESULT res = f_opendir(&dir, path);
    if (res != FR_OK) {
        printf("[FATFS] Failed to open dir: %s, error: %d\n", path, res);
        return;
    }

    while (1) {
        memset(&fno, 0, sizeof(FILINFO)); 
        res = f_readdir(&dir, &fno);

        if (res != FR_OK) {
            printf("[FATFS] Read error: %d\n", res);
            break;
        }

        if (fno.fname[0] == 0) {
            printf("[FATFS] End of dir.\n");
            break;
        }

        // Dump first few raw bytes

        printf((fno.fattrib & AM_DIR) ? "[FATFS] DIR : %s\n" : "[FATFS] FILE: %s (%d bytes)\n",
               fno.fname, (unsigned long long)fno.fsize);
    }

    f_closedir(&dir);
}



void test_fatfs() {

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
}
