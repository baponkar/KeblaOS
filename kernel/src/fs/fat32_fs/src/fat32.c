
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../include/fat32.h"




void fat32_fs_test(int disk_no, uint32_t start_lba, uint32_t sectors){

    printf("\n===== FAT32 TEST START =====\n");

    if(!create_fat32_volume(start_lba, sectors)){
        printf(" Failed to create FAT32 Filesystem at LBA %d", start_lba);
    }
    printf(" Successfully FAT32 Filesystem created at Sector %d\n", start_lba);

    if(!fat32_mount(start_lba)){
        printf(" Failed to Mount Disk %d of DATA Partition\n", disk_no);
    }
    printf(" Successfully Mount Disk %d of DATA Partition\n", disk_no);

    // Directory Test

    FAT32_DIR dir;
    FAT32_DIRENT entry;

    // change directory to root
    if (f_cwd("/")){
        printf("[OK] cwd -> /\n");
    }else{
        printf("[FAIL] cwd root\n");
    }

    // create directories
    if (f_mkdir("/mylongtestdir")){
        printf("[OK] mkdir /mylongtestdir\n");
    }else{
        printf("[FAIL] mkdir /mylongtestdir (maybe exists)\n");
    }

    if (f_mkdir("/mylongtestdir/subdir")){
        printf("[OK] mkdir /mylongtestdir/subdir\n");
    }else{
        printf("[FAIL] mkdir /mylongtestdir/subdir (maybe exists)\n");
    }

    // open directory 
    if (f_opendir(&dir, "/mylongtestdir")){
        printf("[OK] opendir /mylongtestdir\n");
    }else{
        printf("[FAIL] opendir /mylongtestdir\n");
    }

    // list directory
    printf("\nListing /mylongtestdir directory:\n");

    while (f_readdir(&dir, &entry))
    {
        printf("  name:%s  size:%u  cluster:%u  attr:%02X\n",
               entry.name,
               entry.size,
               entry.first_cluster,
               entry.attr);
    }

    f_closedir(&dir);

    // rename directory
    if (f_rename("/mylongtestdir/subdir", "/mylongtestdir/subdir2")){
        printf("[OK] rename subdir -> subdir2\n");
    }else{
        printf("[FAIL] rename directory\n");
    }

    // open directory again
    if (f_opendir(&dir, "/mylongtestdir"))
    {
        printf("\nListing /mylongtestdir after rename:\n");

        while (f_readdir(&dir, &entry))
        {
            printf("  %s\n", entry.name);
        }

        f_closedir(&dir);
    }

    // pattern search
    printf("\nTesting findfirst/findnext (*.TXT):\n");

    if (f_findfirst(&dir, &entry, "/", "*.TXT"))
    {
        do {
            printf("  found: %s\n", entry.name);
        }
        while (f_findnext(&dir, &entry, "*.TXT"));

        f_closedir(&dir);
    }
    else
    {
        printf("  no TXT files found\n");
    }
    
    // File Test
    FAT32_FILE file;
    FAT32_STAT st;

    uint32_t bw, br;

    char buffer[128];
    char line[128];

    // create file
    if (!f_open(&file, "/mylongtestfile.txt", FA_CREATE_ALWAYS | FA_WRITE))
    {
        printf("f_open create failed\n");
        return;
    }
    printf("File created\n");

    // write text
    f_puts("Hello KeblaOS\n", &file);
    f_printf(&file, "Number: %d\n", 12345);

    const char *msg = "Direct write test\n";
    f_write(&file, msg, strlen(msg), &bw);

    printf("Written bytes: %u\n", bw);

    // file position 
    printf("File pos: %u\n", f_tell(&file));

    // sync file
    f_sync(&file);

    // close file
    f_close(&file);

    printf("File closed\n");

    // reopen for read
    if (!f_open(&file, "/mylongtestfile.txt", FA_READ))
    {
        printf("f_open read failed\n");
        return;
    }

    printf("\nReading file:\n");

    while (f_gets(line, sizeof(line), &file))
    {
        printf("%s", line);
    }

    printf("\nEOF reached: %d\n", f_eof(&file));

    printf("File size: %u\n", f_size(&file));

    // seek test
    f_lseek(&file, 0);

    f_read(&file, buffer, 20, &br);
    buffer[br] = '\0';

    printf("\nFirst 20 bytes:\n%s\n", buffer);

    f_close(&file);

    // stat test 
    if (f_stat("/mylongtestfile.txt", &st))
    {
        printf("\nSTAT INFO\n");
        printf("Name: %s\n", st.name);
        printf("Size: %u\n", st.size);
        printf("Cluster: %u\n", st.first_cluster);
        printf("Attr: %02X\n", st.attr);
    }
    else
    {
        printf("stat failed\n");
    }

    // truncate test
    if (f_open(&file, "/mylongtestfile.txt", FA_WRITE))
    {
        f_lseek(&file, 5);
        f_truncate(&file);
        f_close(&file);

        printf("File truncated to 5 bytes\n");
    }

    // unlink test
    if (f_unlink("/mylongtestfile.txt")){
        printf("File deleted\n");
    }else{
        printf("File delete failed\n");
    }

    printf("\n===== FAT32 TEST END =====\n");
}







