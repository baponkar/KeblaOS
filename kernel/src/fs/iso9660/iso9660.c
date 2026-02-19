

#include "../driver/disk/disk.h"

#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include "../lib/string.h"
#include "../lib/ctype.h"

#include "../driver/disk/ahci/satapi.h"

#include "iso9660.h"


#define ISO_BLOCK_SIZE 2048



// -------------------------------------------------------------
// Utility functions
// -------------------------------------------------------------

// Converting little enadian encode into 32 bit value
static uint32_t read_u32_le(const uint8_t *data) {
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

// Converting little endian encode into 16 bit value
static uint16_t read_u16_le(const uint8_t *data) {
    return data[0] | (data[1] << 8);
}

static bool read_iso_block(int disk_no, uint32_t block_no, void *buffer){
    uint8_t lba = block_no * ISO_BLOCK_SIZE;

    if(!kebla_disk_read(disk_no, lba, ISO_BLOCK_SIZE, buffer)){
        return false;
    }

    return true;
}

// Parse directory record filename
static void parse_filename(const iso9660_dir_record_t *record, char *output, size_t output_size) {

    uint8_t name_len = record->file_id_length;

    // Skip '.' and '..' entries
    if (name_len == 1 && (record->file_id[0] == 0x00 || record->file_id[0] == 0x01)) {
        output[0] = '\0';
        return;
    }

    // Handle version suffix (e.g., ";1")
    if (name_len >= 2 && record->file_id[name_len - 2] == ';') {
        name_len -= 2;
    }

    // Copy filename safely
    size_t copy_len = (name_len < output_size - 1) ? name_len : output_size - 1;
    memcpy(output, record->file_id, copy_len);
    output[copy_len] = '\0';

    // Remove ;1
    char *semi = strchr(output, ';');
    if (semi) *semi = '\0';

    // Convert to uppercase (ISO9660 is uppercase-only)
    for (size_t i = 0; i < copy_len; i++) {
        // if (output[i] >= 'a' && output[i] <= 'z') output[i] -= 32;
        output[i] = toupper(output[i]);
    }

}



// -------------------------------------------------------------
// Core ISO9660 functions
// -------------------------------------------------------------

int iso9660_init(int disk_no) {

    if (disk_no >= disk_count || !disks) {
        return -1;
    }

    Disk disk = disks[disk_no];

    if (!disk.context || disk.type != DISK_TYPE_SATAPI){
        printf("ISO9660: Disk %d is not SATAPI type\n", disk_no);
        return -1;
    }

    uint8_t *sector_buffer = (uint8_t *)malloc(ISO_BLOCK_SIZE);
    if(!sector_buffer){
        return -1;
    }

    // Skip first 16 Sectors 
    for (int i = 16; i < 32; i++) {

        if (!kebla_disk_read(disk_no, i, 4, sector_buffer)) {
            printf("ISO9660: Failed to read sector %d\n", i);
            return -1;
        }

        uint8_t vd_type = sector_buffer[0];

        if (vd_type == 0x01) {
            iso9660_pvd_t *pvd = (iso9660_pvd_t *)sector_buffer;

            if (memcmp(pvd->identifier, "CD001", 5) != 0) {
                printf("ISO9660: Invalid volume descriptor\n");
                return -1;
            }

            disks[disk_no].bytes_per_sector = read_u16_le((uint8_t *)&pvd->logical_block_size_le);
            disks[disk_no].total_sectors = read_u32_le((uint8_t *)&pvd->volume_space_size_le);
            disks[disk_no].root_directory_sector = read_u32_le((uint8_t *)&pvd->root_directory_record.extent_location_le);
            disks[disk_no].root_directory_size = read_u32_le((uint8_t *)&pvd->root_directory_record.data_length_le);
            disks[disk_no].pvd_sector = i;

            printf("ISO9660: Initialized disk %d, Volume ID %s, Root LBA %u, Size %u\n",
                 disk_no,  pvd->volume_id, disks[disk_no].root_directory_sector, disks[disk_no].root_directory_size);

            return 0;
        } else if(vd_type == 0xFF){
            break;
        }else{
            break;
        }
    }

    free(sector_buffer);

    printf("ISO9660: No valid Primary Volume Descriptor found\n");

    return -1;
}

int iso9660_mount(int disk_no) {

    if (disk_no >= disk_count || !disks) return -1;

    Disk disk = disks[disk_no];
    if (disk.type != DISK_TYPE_SATAPI) return -1;

    if (iso9660_init(disk_no) != 0) return -1;

    if(!disk.context) return -1;

    if (!satapi_load((HBA_PORT_T *)disk.context)){
        printf("ISO9660: Media load failed!\n");
        return -1;
    }
        
    printf("ISO9660: Mounted ISO9660 on disk %d\n", disk_no);
    
    return 0;
}

int iso9660_unmount(int disk_no) {
    if (disk_no >= disk_count || !disks) return -1;

    Disk disk = disks[disk_no];

    if (disk.type != DISK_TYPE_SATAPI) return -1;

    if(!disk.context) return -1;

    if (!satapi_eject((HBA_PORT_T *)disk.context))
        printf("ISO9660: Media eject failed!\n");

    return 0;
}

// -------------------------------------------------------------
// File/Directory access
// -------------------------------------------------------------

static bool iso9660_find_file(int disk_no, uint32_t dir_sector, uint32_t dir_size, const char *filename, iso9660_file_t *result)
{
    if (disk_no >= disk_count || !disks || dir_sector < 0 || dir_size <= 0 || !filename || !result) return false;
    
    uint8_t *buffer = malloc(dir_size);
    if (!buffer) return false;

    uint32_t sector_size = disks[disk_no].bytes_per_sector;

    if(sector_size <= 0) return false;

    uint32_t sector_count = (dir_size + sector_size - 1) / sector_size;

    if(!kebla_disk_read(disk_no, dir_sector, sector_count, buffer)) {
        free(buffer);
        return false;
    }

    uint8_t *ptr = buffer;
    if(!ptr) return false;
    uint32_t processed = 0;
    bool found = false;

    while (processed < dir_size) {
        iso9660_dir_record_t *record = (iso9660_dir_record_t *)ptr;
        if (record->length == 0) {
            uint32_t skip = sector_size - (processed % sector_size);
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
            result->disk_no = disk_no;
            found = true;
            break;
        }

        ptr += record->length;
        processed += record->length;
    }

    free(buffer);

    return found;
}

int iso9660_stat(int disk_no, char *path, void *fno)
{
    if (disk_no >= disk_count || !disks || !path || !fno){
        return -1;
    }
        

    iso9660_file_t *file_info = (iso9660_file_t *)fno;

    uint32_t cur_sector = disks[disk_no].root_directory_sector;
    uint32_t cur_size   = disks[disk_no].root_directory_size;

    if (path[0] == '/') // Ignoring leading slash
        path++;

    if (path[0] == '\0')
        return -1;

    char *path_copy = strdup(path);
    char *token = strtok(path_copy, "/");

    while (token != NULL) {
        if (!iso9660_find_file(disk_no, cur_sector, cur_size, token, file_info)) {
            free(path_copy);
            return -1;
        }

        token = strtok(NULL, "/");

        if (token != NULL) {
            if (!file_info->is_dir) {
                free(path_copy);
                return -1;
            }

            cur_sector = file_info->sector;
            cur_size   = file_info->size;
        }
    }

    free(path_copy);

    return 0;
}


void *iso9660_open(int disk_no, char *path) {
    if (disk_no >= disk_count || !disks || !path) return NULL;

    if (disks[disk_no].type != DISK_TYPE_SATAPI) return NULL;

    uint32_t cur_sector = disks[disk_no].root_directory_sector;
    uint32_t cur_size = disks[disk_no].root_directory_size;
    iso9660_file_t file_info;

    // Skip leading slash if present
    if (path[0] == '/') {
        path++;
    }

    // Handle root directory case
    if (path[0] == '\0') {
        printf("ISO9660: Cannot open root directory as file\n");
        return NULL;
    }

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
            opened_file->disk_no = disk_no;
            return opened_file;
        }

        token = strtok(NULL, "/");
    }

    free(path_copy);

    return NULL;
}

// #define ISO9660_READ_CHUNK 4096    // 4 KB

// int iso9660_read(void *fp, char *buff, int size) {

//     if(!fp || !buff || size <= 0 || !disks) return -1;

//     iso9660_file_t *file = (iso9660_file_t *)fp;

//     Disk disk = disks[file->disk_no];

//     if (size > file->size)
//         size = file->size;

//     uint32_t total_read = 0;
//     uint32_t remaining = size;
//     uint32_t cur_sector = file->sector;

//     while (remaining > 0) {
//         uint32_t chunk = (remaining > ISO9660_READ_CHUNK) ? ISO9660_READ_CHUNK : remaining;
//         uint32_t sectors = (chunk + disk.bytes_per_sector - 1) / disk.bytes_per_sector;

//         if(disk.bytes_per_sector <= 0) break;

//         if (!kebla_disk_read(file->disk_no, cur_sector, sectors, buff + total_read)){
//             break;
//         }
            
//         total_read += chunk;
//         remaining -= chunk;
//         cur_sector += sectors;
//     }

//     return total_read;
// }

int iso9660_read(void *fp, char *buff, int size) {

    if (!fp || !buff || size <= 0 || !disks) return -1;

    iso9660_file_t *file = (iso9660_file_t *)fp;
    
    Disk disk = disks[file->disk_no];

    if (size > file->size) size = file->size;

    uint32_t total_read = 0;
    uint32_t remaining = size;
    uint32_t cur_sector = file->sector;

    // Temporary sector buffer
    uint8_t *sector_buf = (uint8_t *)malloc(disk.bytes_per_sector);
    if (!sector_buf) return -1;

    while (remaining > 0) {
        if (!kebla_disk_read(file->disk_no, cur_sector, 1, sector_buf)) {
            free(sector_buf);
            return total_read;
        }

        uint32_t copy = (remaining < disk.bytes_per_sector) ? remaining : disk.bytes_per_sector;
        memcpy(buff + total_read, sector_buf, copy);

        total_read += copy;
        remaining -= copy;
        cur_sector++;
    }

    free(sector_buf);
    return total_read;
}


int iso9660_get_fsize(void *fp) {
    if (!fp) return -1;
    iso9660_file_t *file = (iso9660_file_t *)fp;
    return file->size;
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
    if (disk_no >= disk_count || !path || !disks) return NULL;
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

    iso9660_dir_t *dir = (iso9660_dir_t *) malloc(sizeof(iso9660_dir_t));
    if (!dir) return NULL;

    dir->buffer = (uint8_t *) malloc(size);
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



bool iso9660_check_media(void *ctx) {
    HBA_PORT_T *port = (HBA_PORT_T *)ctx;
    if (!port) return false;

    // Check device presence
    if ((port->ssts & 0x0F) != HBA_PORT_DET_PRESENT)
        return false;

    // Additional checks can be added here

    return true;
}

void print_hex(const uint8_t *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", data[i]);

        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}

void hexdump(const void *data, size_t size) {
    const uint8_t *p = data;

    for (size_t i = 0; i < size; i += 16) {
        printf("%08X  ", (unsigned)i);

        for (size_t j = 0; j < 16; j++) {
            if (i + j < size)
                printf("%02X ", p[i + j]);
            else
                printf("   ");
        }

        printf(" ");

        for (size_t j = 0; j < 16 && i + j < size; j++) {
            char c = p[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }

        printf("\n");
    }
}


void iso9660_test(int disk_no, char *test_path) {

    // Initial Critical Testing
    if (disk_no >= disk_count || !disks) {
        printf("ISO9660 Test: Invalid Disk No %d!\n", disk_no);
        return;
    }

    if(!test_path){
        printf("ISO9600_Test: Null test_path pointer!\n");
        return;
    }

    Disk disk = disks[disk_no];

    if(disk.type != DISK_TYPE_SATAPI) {
        printf("ISO9660 Test: Disk %d is not SATAPI!\n", disk_no);
        return;
    }

    // Mount the ISO9660 FS 
    if(iso9660_mount(disk_no) != 0){
        printf("ISO9660 Test: Failed to mount the disk %d!\n");
        return;
    }
    printf("ISO9660 Test: Successfully mounted Disk %d\n", disk_no);

    // Checking presence of the file
    iso9660_file_t file_info;
    if(iso9660_stat(disk_no, test_path, &file_info) != 0){
        printf("ISO9660 Test: File %s is not present!\n", test_path);
        return;
    }
    printf("ISO9660 Test: File %s is present.\n", test_path);

    // Opening the above file
    void *file_ptr = iso9660_open(disk_no, test_path);
    if(!file_ptr){
        printf("ISO9660 Test: Failed to open %s!\n", test_path);
        return;
    }
    printf("ISO9660 Test: Successfully  open %s!\n", test_path);

    // Getting File size
    int file_size = iso9660_get_fsize(file_ptr);
    if(file_size <= 0){
        printf("ISO9660 Test: Invalid To Get File Size!\n");
        return;
    }
    printf("ISO9660 Test: Successfully Get File Size: %d\n", file_size);

    // Reading the opened file
    char *buff = malloc(file_size);
    if(!buff){
        printf("ISO9660 Test: Memory allocation is failed!\n");
        return;
    }

    // Reading the file
    int rb = iso9660_read(file_ptr, buff, file_size);
    if(rb  <= 0){
        printf("ISO9660 Test: Readig file %s is failed!\n", test_path);
        free(buff);
        return;
    }
    printf("ISO9660 Test: Sucessfully read %d bytes from %s!\n", rb, test_path);

    printf("ISO9660 Test: %s Content: %s.\n", test_path, buff);

    hexdump(buff, file_size);

    free(buff);
}


