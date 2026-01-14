#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>


bool write_limine_stage2_embedding(int disk_no);
bool write_fat32_fs_at_2048(int disk_no);

bool is_os_installed(int boot_pd, int ld);
int uefi_install(int iso_pd, int boot_pd);

int create_user_dirs(int pd);
void init_user_space(int boot_pd, int user_ld);



