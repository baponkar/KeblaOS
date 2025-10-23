#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>


#pragma pack(push, 1)

// ISO9660 Directory Record
typedef struct {
    uint8_t length;
    uint8_t ext_attr_length;
    uint32_t extent_location_le;
    uint32_t extent_location_be;
    uint32_t data_length_le;
    uint32_t data_length_be;
    uint8_t date_time[7];
    uint8_t file_flags;
    uint8_t file_unit_size;
    uint8_t interleave_gap_size;
    uint16_t volume_sequence_number_le;
    uint16_t volume_sequence_number_be;
    uint8_t file_id_length;
    char file_id[1];            // Actually variable length
} iso9660_dir_record_t;


// ISO9660 Primary Volume Descriptor
typedef struct {
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
} iso9660_pvd_t;
#pragma pack(pop)


// File info structure
typedef struct {
    char name[256];
    uint32_t sector;
    uint32_t size;
    int disk_no;
    bool is_dir;
} iso9660_file_t;


// Directory iterator structure
typedef struct {
    uint8_t *buffer;
    uint32_t size;
    uint32_t offset;
    int disk_no;
} iso9660_dir_t;


int iso9660_init(int disk_no);

int iso9660_mount(int disk_no);
int iso9660_unmount(int disk_no);

void *iso9660_open(int disk_no, char *path, int mode);
int iso9660_read(void *fp, char *buff, int size);
int iso9660_close(void *fp);


void *iso9660_opendir(int disk_no, char *path);
int iso9660_readdir(void *dirp, iso9660_file_t *entry);
int iso9660_closedir(void *dirp);

