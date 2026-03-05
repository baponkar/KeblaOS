
#include "fat32_test_1.h"
#include "diskio_test_1.h"

#include "main.h"





int main() {

    // if(!diskio_test(0)) {
    //     printf("Disk I/O test failed\n");
    //     return 1;
    // }
    // printf("Disk I/O test passed successfully!\n");


    if (!fat32_test()) {
        printf("FAT32 test failed\n");
        return 1;
    }
    printf("FAT32 test passed successfully!\n");

    return 0;
}


