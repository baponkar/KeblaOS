

#include "../driver/disk/disk.h"

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"

#include "../driver/disk/ahci/satapi.h"

#include "iso9660.h"




// -------------------------------------------------------------
// Utility functions
// -------------------------------------------------------------

static uint32_t read_u32_le(const uint8_t *data) {
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

static uint16_t read_u16_le(const uint8_t *data) {
    return data[0] | (data[1] << 8);
}

// Parse directory record filename
static void parse_filename(const iso9660_dir_record_t *record, char *output, size_t output_size) {
    uint8_t name_len = record->file_id_length;

    // Handle version suffix (e.g., ";1")
    if (name_len >= 2 && record->file_id[name_len - 2] == ';') {
        name_len -= 2;
    }

    size_t copy_len = (name_len < output_size - 1) ? name_len : output_size - 1;
    memcpy(output, record->file_id, copy_len);
    output[copy_len] = '\0';

    // Replace weird chars
    for (size_t i = 0; i < copy_len; i++) {
        if (output[i] == '\0' || output[i] == '\x01')
            output[i] = '_';
    }
}

// -------------------------------------------------------------
// Core ISO9660 functions
// -------------------------------------------------------------

int iso9660_init(int disk_no) {
    if (disk_no >= disk_count) return false;

    Disk disk = disks[disk_no];
    if (!disk.context || disk.type != DISK_TYPE_SATAPI)
        return false;

    if (!kebla_disk_init(disk_no)) {
        printf("ISO9660: Failed to initialize %d\n", disk_no);
        return false;
    }

    uint8_t sector_buffer[2048];

    for (int i = 16; i < 32; i++) {
        if (!kebla_disk_read(disk_no, i, 1, sector_buffer)) {
            printf("ISO9660: Failed to read sector %d\n", i);
            return false;
        }

        uint8_t vd_type = sector_buffer[0];
        if (vd_type == 0x01) {
            iso9660_pvd_t *pvd = (iso9660_pvd_t *)sector_buffer;

            if (memcmp(pvd->identifier, "CD001", 5) != 0) {
                printf("ISO9660: Invalid volume descriptor\n");
                return false;
            }

            disks[disk_no].bytes_per_sector = read_u16_le((uint8_t *)&pvd->logical_block_size_le);
            disks[disk_no].total_sectors = read_u32_le((uint8_t *)&pvd->volume_space_size_le);
            disks[disk_no].root_directory_sector = read_u32_le((uint8_t *)&pvd->root_directory_record.extent_location_le);
            disks[disk_no].root_directory_size = read_u32_le((uint8_t *)&pvd->root_directory_record.data_length_le);
            disks[disk_no].pvd_sector = i;

            printf("ISO9660: Mounted disk-%d\n", disk_no);
            printf("  Volume: %s\n", pvd->volume_id);
            printf("  Root: LBA=%u size=%u\n",
                   disks[disk_no].root_directory_sector,
                   disks[disk_no].root_directory_size);
            return true;
        }

        if (vd_type == 0xFF)
            break;
    }

    printf("ISO9660: No valid Primary Volume Descriptor found\n");
    return false;
}

int iso9660_mount(int disk_no) {
    if (disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];
    if (disk.type != DISK_TYPE_SATAPI) return -1;

    if (!iso9660_init(disk_no)) return -1;

    if (!satapi_load((HBA_PORT_T *)disk.context))
        printf("ISO9660: Media load failed!\n");

    return 0;
}

int iso9660_unmount(int disk_no) {
    if (disk_no >= disk_count) return -1;

    Disk disk = disks[disk_no];
    if (disk.type != DISK_TYPE_SATAPI) return -1;

    if (!satapi_eject((HBA_PORT_T *)disk.context))
        printf("ISO9660: Media eject failed!\n");

    return 0;
}

// -------------------------------------------------------------
// File/Directory access
// -------------------------------------------------------------

static bool iso9660_find_file(
    int disk_no,
    uint32_t dir_sector,
    uint32_t dir_size,
    const char *filename,
    iso9660_file_t *result)
{
    Disk disk = disks[disk_no];
    uint8_t *buffer = malloc(dir_size);
    if (!buffer) return false;

    uint32_t sector_count = (dir_size + disk.bytes_per_sector - 1) / disk.bytes_per_sector;
    if (!kebla_disk_read(disk_no, dir_sector, sector_count, buffer)) {
        free(buffer);
        return false;
    }

    uint8_t *ptr = buffer;
    uint32_t processed = 0;
    bool found = false;

    while (processed < dir_size) {
        iso9660_dir_record_t *record = (iso9660_dir_record_t *)ptr;
        if (record->length == 0) {
            uint32_t skip = disk.bytes_per_sector - (processed % disk.bytes_per_sector);
            ptr += skip;
            processed += skip;
            continue;
        }

        if (record->file_id_length == 1 && (record->file_id[0] == 0x00 || record->file_id[0] == 0x01)) {
            ptr += record->length;
            processed += record->length;
            continue;
        }

        char current_filename[256];
        parse_filename(record, current_filename, sizeof(current_filename));

        if (strcmp(current_filename, filename) == 0) {
            strcpy(result->name, current_filename);
            result->sector = read_u32_le((uint8_t *)&record->extent_location_le);
            result->size = read_u32_le((uint8_t *)&record->data_length_le);
            result->is_dir = (record->file_flags & 0x02) != 0;
            found = true;
            break;
        }

        ptr += record->length;
        processed += record->length;
    }

    free(buffer);
    return found;
}

void *iso9660_open(int disk_no, char *path, int mode) {
    if (disk_no >= disk_count) return NULL;

    Disk disk = disks[disk_no];
    if (disk.type != DISK_TYPE_SATAPI) return NULL;

    uint32_t cur_sector = disk.root_directory_sector;
    uint32_t cur_size = disk.root_directory_size;
    iso9660_file_t file_info;

    char *path_copy = strdup(path);
    char *token = strtok(path_copy, "/");

    while (token != NULL) {
        if (!iso9660_find_file(disk_no, cur_sector, cur_size, token, &file_info)) {
            printf("ISO9660: '%s' not found!\n", token);
            free(path_copy);
            return NULL;
        }

        if (file_info.is_dir) {
            cur_sector = file_info.sector;
            cur_size = file_info.size;
        } else {
            free(path_copy);
            iso9660_file_t *opened_file = malloc(sizeof(iso9660_file_t));
            if (!opened_file) return NULL;
            memcpy(opened_file, &file_info, sizeof(iso9660_file_t));
            return opened_file;
        }

        token = strtok(NULL, "/");
    }

    free(path_copy);
    return NULL;
}

int iso9660_read(void *fp, char *buff, int size) {
    if (!fp || !buff || size <= 0) return -1;
    iso9660_file_t *file = (iso9660_file_t *)fp;


    Disk disk = disks[file->disk_no];

    if (size > file->size)
        size = file->size;

    uint32_t sector_count = (size + disk.bytes_per_sector - 1) / disk.bytes_per_sector;
    if (!kebla_disk_read(0, file->sector, sector_count, buff)) {
        printf("ISO9660: Failed to read sector %u\n", file->sector);
        return -1;
    }
    return size;
}

int iso9660_close(void *fp) {
    if (!fp) return -1;
    free(fp);
    return 0;
}

// -------------------------------------------------------------
// Directory iterator support
// -------------------------------------------------------------

void *iso9660_opendir(int disk_no, char *path) {
    iso9660_file_t dir_info;

    uint32_t sector = disks[disk_no].root_directory_sector;
    uint32_t size = disks[disk_no].root_directory_size;

    if (path && strlen(path) > 0 && strcmp(path, "/") != 0) {
        if (!iso9660_find_file(disk_no, sector, size, path, &dir_info))
            return NULL;

        if (!dir_info.is_dir)
            return NULL;

        sector = dir_info.sector;
        size = dir_info.size;
    }

    iso9660_dir_t *dir = malloc(sizeof(iso9660_dir_t));
    if (!dir) return NULL;

    dir->buffer = malloc(size);
    if (!dir->buffer) {
        free(dir);
        return NULL;
    }

    kebla_disk_read(disk_no, sector, (size + 2047) / 2048, dir->buffer);
    dir->size = size;
    dir->offset = 0;

    return dir;
}

int iso9660_readdir(void *dirp, iso9660_file_t *entry) {
    iso9660_dir_t *dir = (iso9660_dir_t *)dirp;

    while (dir->offset < dir->size) {
        iso9660_dir_record_t *record = (iso9660_dir_record_t *)(dir->buffer + dir->offset);

        if (record->length == 0) {
            uint32_t skip = 2048 - (dir->offset % 2048);
            dir->offset += skip;
            continue;
        }

        dir->offset += record->length;

        if (record->file_id_length == 1 && 
            (record->file_id[0] == 0x00 || record->file_id[0] == 0x01))
            continue;

        parse_filename(record, entry->name, sizeof(entry->name));
        entry->sector = read_u32_le((uint8_t *)&record->extent_location_le);
        entry->size = read_u32_le((uint8_t *)&record->data_length_le);
        entry->is_dir = (record->file_flags & 0x02) != 0;

        return 1; // Entry found
    }

    return 0; // End of directory
}

int iso9660_closedir(void *dirp) {
    iso9660_dir_t *dir = (iso9660_dir_t *)dirp;
    if (!dir) return -1;
    free(dir->buffer);
    free(dir);
    return 0;
}

