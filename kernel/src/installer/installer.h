#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>


bool write_limine_stage2_embedding(int disk_no);
bool write_fat32_fs_at_2048(int disk_no);

bool is_os_installed(int boot_pd, int ld);
int uefi_install(int iso_pd, int boot_pd);

int create_user_dirs(int pd);
void init_user_space(int boot_pd, int boot_ld, int user_ld);



typedef enum {
    ENC_UNKNOWN = 0,
    ENC_ASCII = 1,
    ENC_UTF8 = 2,
    ENC_UTF8_BOM = 3,
    ENC_UTF16_LE = 4,
    ENC_UTF16_BE = 5,
    ENC_UTF32_LE = 6,
    ENC_UTF32_BE = 7
} text_encoding_t;

text_encoding_t detect_encoding(const uint8_t *buf, size_t size);
int change_file_encoding(const uint8_t *buf, size_t size, text_encoding_t enc_type,  uint8_t **out_buf, size_t *out_size);



static int mark_efi_system_partition(int disk_no);