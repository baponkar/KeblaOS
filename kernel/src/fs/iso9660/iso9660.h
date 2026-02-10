#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>



// ISO9660 Directory Record
typedef struct __attribute__((packed)){
    uint8_t length;                     // Size of the entire directory record in bytes.
    uint8_t ext_attr_length;            // Length of extended attribute record.
    uint32_t extent_location_le;        // “Where the file data starts on disk (LBA).”
    uint32_t extent_location_be;        // “Where the file data starts on disk (LBA).”
    uint32_t data_length_le;            // Data length (file size)
    uint32_t data_length_be;            // Data length (file size)
    uint8_t date_time[7];               // ISO9660 directory timestamp format:
    uint8_t file_flags;                 // File flags : hidden: 0x1, directory: 0x2, associated file 0x4, record format 0x8, permissions 0x10 multi-extent
    uint8_t file_unit_size;
    uint8_t interleave_gap_size;
    uint16_t volume_sequence_number_le;
    uint16_t volume_sequence_number_be;
    uint8_t file_id_length;             // Length of the filename.
    char file_id[1];                    // Actually variable length
} iso9660_dir_record_t; // 34 Bytes : Actual length : 33 + file_id_length (+ padding)



// ISO9660 Primary Volume Descriptor
typedef struct __attribute__((packed)){
    uint8_t type;                   // 0x01 for Primary Volume Descriptor
    char identifier[5];             // "CD001"
    uint8_t version;                // 0x01
    uint8_t unused1;
    char system_id[32];
    char volume_id[32];
    uint8_t unused2[8];
    uint32_t volume_space_size_le;  // Little-endian
    uint32_t volume_space_size_be;  // Big-endian
    uint8_t unused3[32];
    uint16_t volume_set_size_le;
    uint16_t volume_set_size_be;
    uint16_t volume_sequence_number_le;
    uint16_t volume_sequence_number_be;
    uint16_t logical_block_size_le;
    uint16_t logical_block_size_be;
    uint32_t path_table_size_le;
    uint32_t path_table_size_be;
    uint32_t path_table_location_le;
    uint32_t path_table_location_be;
    uint32_t optional_path_table_location_le;
    uint32_t optional_path_table_location_be;
    iso9660_dir_record_t root_directory_record;
    char volume_set_id[128];
    char publisher_id[128];
    char data_preparer_id[128];
    char application_id[128];
    char copyright_file_id[38];
    char abstract_file_id[36];
    char bibliographic_file_id[37];
    char creation_date[17];
    char modification_date[17];
    char expiration_date[17];
    char effective_date[17];
    uint8_t file_structure_version;
    uint8_t unused4;
    uint8_t application_data[512];
    uint8_t reserved[653];
} iso9660_pvd_t;    // 2048 bytes



// File info structure
typedef struct {
    char name[256];
    uint32_t sector;
    uint32_t size;
    int disk_no;
    bool is_dir;
} iso9660_file_t;   // 272 bytes


// Directory iterator structure
typedef struct {
    uint8_t *buffer;
    uint32_t size;
    uint32_t offset;
    int disk_no;    // 8 bytes
} iso9660_dir_t;    // 24 bytes


int iso9660_init(int disk_no);

bool iso9660_check_media(void *ctx);

int iso9660_mount(int disk_no);
int iso9660_unmount(int disk_no);

int iso9660_stat(int disk_no, char *path, void *fno);
void *iso9660_open(int disk_no, char *path);
int iso9660_read(void *fp, char *buff, int size);
int iso9660_get_fsize(void *fp);
int iso9660_close(void *fp);


void *iso9660_opendir(int disk_no, char *path);
int iso9660_readdir(void *dirp, iso9660_file_t *entry);
int iso9660_closedir(void *dirp);



void iso9660_test(int disk_no, char *test_path);

