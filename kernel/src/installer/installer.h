#pragma once
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#pragma pack(push, 1)

// ISO9660 Directory Record
typedef struct {
    uint8_t  length;
    uint8_t  ext_attr_length;
    uint32_t extent_location_le;
    uint32_t extent_location_be;
    uint32_t data_length_le;
    uint32_t data_length_be;
    uint8_t  date_time[7];
    uint8_t  file_flags;
    uint8_t  file_unit_size;
    uint8_t  interleave_gap_size;
    uint16_t volume_sequence_number_le;
    uint16_t volume_sequence_number_be;
    uint8_t  file_id_length;
    char     file_id[1];  // Actually variable length
} iso9660_dir_record_t;

// ISO9660 Primary Volume Descriptor
typedef struct {
    uint8_t  type;                       // 0x01 for Primary Volume Descriptor
    char     identifier[5];              // "CD001"
    uint8_t  version;                    // 0x01
    uint8_t  unused1;
    char     system_id[32];
    char     volume_id[32];
    uint8_t  unused2[8];
    uint32_t volume_space_size_le;       // Little-endian
    uint32_t volume_space_size_be;       // Big-endian
    uint8_t  unused3[32];
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
    char     volume_set_id[128];
    char     publisher_id[128];
    char     data_preparer_id[128];
    char     application_id[128];
    char     copyright_file_id[38];
    char     abstract_file_id[36];
    char     bibliographic_file_id[37];
    char     creation_date[17];
    char     modification_date[17];
    char     expiration_date[17];
    char     effective_date[17];
    uint8_t  file_structure_version;
    uint8_t  unused4;
    uint8_t  application_data[512];
    uint8_t  reserved[653];
} iso9660_pvd_t;

#pragma pack(pop)



// Parsed ISO9660 File Entry
typedef struct {
    char     name[256];
    uint32_t sector;
    uint32_t size;
    bool     is_dir;
} iso9660_file_t;


bool is_keblaos_installed(int disk_no);
void uefi_install(int boot_disk_no, int iso_disk_no);
