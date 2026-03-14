#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef struct partition{
    int pd_no;
    int ld_no;
    uint64_t start_lba;
    uint64_t sectors;

    guid_t guid;
    guid_t type_guid;
    char name[64];
} partition_t;


bool uefi_install(int boot_disk_no, int main_disk_no, uint64_t esp_start_lba, uint64_t esp_sectors, uint64_t total_sectors);
bool verify_installation(int disk_no, uint32_t start_lba);










