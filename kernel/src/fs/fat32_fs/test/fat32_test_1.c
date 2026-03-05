

#include  <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/fat32.h"

#include "../include/guid.h"
#include "../include/gpt.h"
#include "../include/partition_manager.h"

#include "../test/fat32_test_1.h"

#define PDRV_NO 0

#define SECTOR_SIZE 512

#define TOTAL_SECTORS 1 * 1024 * 1024 * 1024 / SECTOR_SIZE  // 1 GB

#define PARTITION_1_START_LBA 2048
#define PARTITION_1_TOTAL_LBA 100 * 1024 * 1024 / SECTOR_SIZE    // 100 MB
#define PARTITION_1_NAME "BOOT PARTITION"

#define PARTITION_2_START_LBA PARTITION_1_START_LBA + PARTITION_1_TOTAL_LBA // Next to Partition 1
#define PARTITION_2_TOTAL_LBA TOTAL_SECTORS - PARTITION_1_START_LBA - PARTITION_1_TOTAL_LBA - 2048 // Rest of Disk Space, 2048 for safety
#define PARTITION_2_NAME "DATA PARTITION"



// Testing 8.3 Filename FAT32 Test
bool fat32_test(){


    if(create_partition(PDRV_NO, PARTITION_1_START_LBA, PARTITION_1_TOTAL_LBA, ESP_GUID, ESP_TYPE_GUID, PARTITION_1_NAME)){
        printf("Successfully created Partition at Sector %d\n", PARTITION_1_START_LBA);
    }
    
    if(create_partition( PDRV_NO, PARTITION_2_START_LBA, PARTITION_2_TOTAL_LBA, DATA_PARTITION_GUID, LINUX_FS_GUID, PARTITION_2_NAME)){
        printf("Successfully created Partition at Sector %d\n",  PARTITION_2_START_LBA);
    }


    if (!create_fat32_volume( PARTITION_1_START_LBA, PARTITION_1_TOTAL_LBA)) {
        printf("Failed to create FAT32 volume\n");
        return 1;
    }
    printf("[FAT32 TEST] Successfully created FAT32 volume at LBA: %d with size: %d MB\n", PARTITION_1_START_LBA, PARTITION_1_TOTAL_LBA);

    if (!create_fat32_volume( PARTITION_2_START_LBA, PARTITION_2_TOTAL_LBA)) {
        printf("Failed to create FAT32 volume\n");
        return 1;
    }
    printf("[FAT32 TEST] Successfully created FAT32 volume at LBA: %d with size: %d MB\n", PARTITION_1_START_LBA, PARTITION_2_TOTAL_LBA);

    if(!fat32_mount(PARTITION_1_START_LBA)){
        printf("[FAT32 TEST] Failed to Mount FAT32 FS at LBA: %d!\n", PARTITION_1_START_LBA);
        return false;
    }
    printf("[FAT32 TEST] Successfully Mount Disk.\n");

    // Creating a short directory at root
    if(!fat32_mkdir("BOOT")){
        printf("[FAT32 TEST] Creating Directory %s is failed!\n", "BOOT");
        return false;
    }

    // Creating a short file at that BOOT
    

    // Crating a long directory at root
    char *dir_path = "mylongtestdir";
    if(!fat32_mkdir( dir_path)){
        printf("[FAT32 TEST] Creating Directory %s is failed!\n", dir_path);
        return false;
    }
    printf("[FAT32 TEST] Creating Directory %s is success.\n", dir_path);

    // Finding Directory Cluster no
    uint32_t dir_cluster_no = 0;
    if(!fat32_path_to_cluster( dir_path, &dir_cluster_no)){
        printf("[FAT32 TEST] Failed to get Cluster no for %s\n", dir_path);
        return false;
    }
    printf("[FAT32 TEST] Successfully get Cluster no %d for directory %s\n", dir_cluster_no, dir_path);

    // Creating mylongtestfile.text
    char *file_name = "mylongtestfile.text";   // 8.3 Short Filename
    char *buff = "This is a test text string for testing fat32 filesystem.";
    uint32_t file_size = strlen(buff);

    if(!fat32_create_file_in_dir( dir_cluster_no, file_name, buff, file_size)){
        return false;
    }
    printf("[FAT32 TEST] Successfully created %s\n\n", file_name);

    // Opening testfile.txt
    const char *file_path = "mylongtestdir/mylongtestfile.text";
    FAT32_FILE file;
    if(!fat32_open( file_path, &file)){
        printf("[FAT32 TEST] Faile to read file %s\n", file_path);
        return false;
    }
    printf("[FAT32 TEST] Successfully open file %s\n", file_path);

    // Reading testfile.txt 
    char *buffer = (char *) malloc(file_size);
    uint32_t rb = fat32_read( &file, buffer, file_size);
    if(rb <= 0){
        printf("[FAT32 TEST] Failed to read file %s!\n", file_path);
        // return false;
    }
    printf("[FAT32 TEST] Successfully read %d bytes\n", rb);
    printf("[FAT32 TEST] File content: %s\n", buffer);

    free(buffer);

    return true;
}


