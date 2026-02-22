
#include "../driver/vga/vga_term.h"

#include "../../driver/disk/disk.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/stdlib.h"
#include "../../lib/time.h"
#include "../../lib/ctype.h"

#include "fat32_utility.h"



extern uint64_t fat32_base_lba;             // Defined in fat32
extern BPB *bpb;                            // Defined in fat32

uint32_t fat32_cwd_cluster = 2;             // Current Working Directory Cluster

static uint32_t fat32_free_cluster_no = 2;


static bool fat32_read_sector(int disk_no, uint64_t lba, void *buf) {
    return kebla_disk_read(disk_no, fat32_base_lba + lba, 1, buf);
}

static bool fat32_write_sector(int disk_no, uint64_t lba, const void *buf) {
    return kebla_disk_write(disk_no, fat32_base_lba + lba, 1, (void*)buf);
}

bool fat32_read_cluster(int disk_no, uint32_t cluster_number, void *buffer){
    uint32_t first_sector = get_first_sector_of_cluster(cluster_number);
    uint8_t *buf_ptr = (uint8_t *)buffer;
    
    for(uint8_t i = 0; i < bpb->BPB_SecPerClus; i++){
        if(!fat32_read_sector(disk_no, first_sector + i, buf_ptr + (i * bpb->BPB_BytsPerSec))){
            return false;
        }
    }
    return true;
}

bool fat32_read_cluster_chain(int disk_no, uint32_t start_cluster, void *buffer, uint32_t max_bytes) {

    uint8_t *buf = (uint8_t *)buffer;
    uint32_t current = start_cluster;
    uint32_t bytes_read = 0;
    uint32_t cluster_size = get_cluster_size_bytes();

    while (is_valid_cluster(current)) {
        if (bytes_read + cluster_size > max_bytes) {
            return false; // Buffer too small
        }

        if (!fat32_read_cluster(disk_no, current, buf + bytes_read)) {
            return false;
        }

        bytes_read += cluster_size;

        uint32_t next = fat32_get_next_cluster(disk_no, current);
        if (is_end_of_cluster_chain(next)) {
            break;
        }

        current = next;
    }

    return true;
}



bool fat32_write_cluster(int disk_no, uint32_t cluster_number, const void *buffer)
{
    uint32_t first_sector = get_first_sector_of_cluster(cluster_number);

    return kebla_disk_write(disk_no, fat32_base_lba + first_sector, bpb->BPB_SecPerClus, (void*)buffer );
}


bool fat32_write_cluster_chain( int disk_no, const void *buffer, uint32_t size, uint32_t *first_cluster)
{
    const uint8_t *buf = (const uint8_t *)buffer;
    uint32_t cluster_size = get_cluster_size_bytes();
    uint32_t written = 0;

    uint32_t prev_cluster = 0;
    uint32_t curr_cluster = 0;

    uint8_t *temp = malloc(cluster_size);
    if (!temp) return false;


    while (written < size) {
        if (!fat32_allocate_cluster(disk_no, &curr_cluster)) {
            free(temp);
            return false;
        }

        if (prev_cluster != 0) {
            fat32_set_next_cluster(disk_no, prev_cluster, curr_cluster);
        } else {
            *first_cluster = curr_cluster;
        }

        uint32_t to_write =  (size - written > cluster_size)  ? cluster_size : (size - written);

        // memset(temp, 0, cluster_size);
        // memcpy(temp, buf + written, to_write);
        if (to_write == cluster_size) {
            if(!fat32_write_cluster(disk_no, curr_cluster, buf + written)){
                free(temp);
                return false;
            }
        } else {
            memset(temp, 0, cluster_size);
            memcpy(temp, buf + written, to_write);
            if(!fat32_write_cluster(disk_no, curr_cluster, temp)){
                free(temp);
                return false;
            }
        }

        // if (!fat32_write_cluster(disk_no, curr_cluster, temp)) {
        //     free(temp);
        //     return false;
        // }

        written += to_write;

        prev_cluster = curr_cluster;
    }

    fat32_set_next_cluster(disk_no, prev_cluster, 0x0FFFFFFF);

    free(temp);

    return true;
}


// Clearing a single cluster
bool fat32_clear_cluster(int disk_no, uint32_t cluster) {
    uint32_t cluster_size = bpb->BPB_BytsPerSec * bpb->BPB_SecPerClus;
    uint8_t *zero = malloc(cluster_size);
    if (!zero) return false;

    memset(zero, 0, cluster_size);
    bool ok = fat32_write_cluster(disk_no, cluster, zero);
    free(zero);
    return ok;
}

bool fat32_free_cluster_chain(int disk_no, uint32_t start_cluster){
    uint32_t current_cluster = start_cluster;

    while (is_valid_cluster(current_cluster)) {
        uint32_t next_cluster = fat32_get_next_cluster(disk_no, current_cluster);
        if (next_cluster == 0) {
            return false; // Error reading next cluster
        }

        // Mark current cluster as free
        if (!fat32_set_next_cluster(disk_no, current_cluster, 0x00000000)) {
            return false; // Error setting cluster
        }

        if (is_end_of_cluster_chain(next_cluster)) {
            break; // Reached end of chain
        }

        current_cluster = next_cluster;
    }

    return true;
}

bool fat32_set_volume_label(int disk_no, const char *label) {
    uint32_t root_cluster = bpb->BPB_RootClus;
    uint32_t cluster_size = get_cluster_size_bytes();
    
    uint8_t *buf = (uint8_t *) malloc(cluster_size);
    if (!buf) return false;

    // 1. Read the first cluster of the root directory
    if (!fat32_read_cluster(disk_no, root_cluster, buf)) {
        free(buf);
        return false;
    }

    // 2. Find an empty slot or an existing Volume ID slot
    DirEntry *target_entry = NULL;
    for (uint32_t offset = 0; offset < cluster_size; offset += 32) {
        DirEntry *entry = (DirEntry *)(buf + offset);
        
        // If we find an existing label, overwrite it. 
        // Otherwise, take the first available slot (0x00 or 0xE5).
        if (entry->DIR_Attr == ATTR_VOLUME_ID || entry->DIR_Name[0] == 0x00 || entry->DIR_Name[0] == 0xE5) {
            target_entry = entry;
            break;
        }
    }

    if (!target_entry) {
        free(buf);
        return false; // Root cluster is full (unlikely for a fresh disk)
    }

    // 3. Setup the Label Entry
    memset(target_entry, 0, sizeof(DirEntry));
    
    // Format name to 11 chars, no dot, uppercase, space padded
    for (int i = 0; i < 11; i++) {
        if (label[i] && label[i] != '.') {
            target_entry->DIR_Name[i] = toupper(label[i]);
        } else {
            target_entry->DIR_Name[i] = ' ';
        }
    }

    target_entry->DIR_Attr = ATTR_VOLUME_ID;
    target_entry->DIR_FstClusHI = 0; // Always 0 for Volume Labels
    target_entry->DIR_FstClusLO = 0; // Always 0 for Volume Labels
    target_entry->DIR_FileSize = 0;

    // 4. Write back to disk
    bool ok = fat32_write_cluster(disk_no, root_cluster, buf);
    free(buf);
    return ok;
}


uint32_t fat32_get_next_cluster(int disk_no, uint32_t current_cluster){
    uint32_t fat_offset = current_cluster * 4; // Total bytes as Each FAT32 entry is 4 bytes
    uint32_t fat_sector_number = bpb->BPB_RsvdSecCnt + (fat_offset / bpb->BPB_BytsPerSec);
    uint32_t ent_offset = fat_offset % bpb->BPB_BytsPerSec; // Sector

    uint8_t sector_buffer[512];
    if (!fat32_read_sector(disk_no, fat_sector_number, sector_buffer)) {
        return 0;               // Error reading sector
    }

    uint32_t next_cluster = *(uint32_t *)&sector_buffer[ent_offset];    // As Current Cluster's FAT wrote next Cluster number
    next_cluster &= 0x0FFFFFFF; // Mask to get the lower 28 bits

    return next_cluster;
}

bool fat32_set_next_cluster(int disk_no, uint32_t current_cluster, uint32_t next_cluster) {
    uint32_t fat_offset = current_cluster * 4;
    uint32_t fat_sector_relative = fat_offset / bpb->BPB_BytsPerSec;
    uint32_t ent_offset = fat_offset % bpb->BPB_BytsPerSec;

    uint8_t sector_buffer[512]; // Note: Ideally use bpb->BPB_BytsPerSec

    // 1. Read from FAT1 to get the current entry (to preserve high 4 bits)
    uint32_t fat1_sector = bpb->BPB_RsvdSecCnt + fat_sector_relative;

    if (!fat32_read_sector(disk_no, fat1_sector, sector_buffer)) 
        return false;

    // 2. Update the entry
    uint32_t *entry = (uint32_t *)&sector_buffer[ent_offset];
    *entry = (*entry & 0xF0000000) | (next_cluster & 0x0FFFFFFF);

    // 3. Write this same sector to ALL FAT tables
    for (uint8_t i = 0; i < bpb->BPB_NumFATs; i++) {
        uint32_t target_sector = bpb->BPB_RsvdSecCnt + (i * bpb->BPB_FATSz32) + fat_sector_relative;
        if (!fat32_write_sector(disk_no, target_sector, sector_buffer)) {
            return false;
        }
    }
    return true;
}



// Allocate a free cluster and return its number
bool fat32_allocate_cluster(int disk_no, uint32_t *allocated_cluster)
{
    uint32_t total_clusters = get_total_clusters();

    for (uint32_t cluster = fat32_free_cluster_no; cluster < total_clusters + 2; cluster++)
    {
        uint32_t fat_offset = cluster * 4;  // Each entry is 4 bytes
        uint32_t fat_sector_number = bpb->BPB_RsvdSecCnt + (fat_offset / bpb->BPB_BytsPerSec);
        uint32_t ent_offset =  fat_offset % bpb->BPB_BytsPerSec;

        uint8_t sector_buffer[512];

        if (!fat32_read_sector(disk_no, fat_sector_number, sector_buffer))
            return false;

        uint32_t entry = *(uint32_t *)&sector_buffer[ent_offset] & 0x0FFFFFFF;

        if (entry == 0) {
            fat32_set_next_cluster(disk_no, cluster, 0x0FFFFFFF);
            *allocated_cluster = cluster;

            fat32_free_cluster_no = cluster + 1;    // update fat32_free_cluster_no

            return true;
        }
    }

    return false;
}



bool fat32_allocate_cluster_chain(int disk_no, uint32_t count, uint32_t *first_cluster){
    uint32_t prev_cluster = 0;
    *first_cluster = 0;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t new_cluster;
        if (!fat32_allocate_cluster(disk_no, &new_cluster)) {
            return false; // Allocation failed
        }

        if (prev_cluster != 0) {
            if (!fat32_set_next_cluster(disk_no, prev_cluster, new_cluster)) {
                return false; // Error linking clusters
            }
        } else {
            *first_cluster = new_cluster; // Set first cluster
        }

        prev_cluster = new_cluster;
    }

    // Mark the last cluster as end of chain
    if (!fat32_set_next_cluster(disk_no, prev_cluster, 0x0FFFFFFF)) {
        return false; // Error setting end of chain
    }

    return true;
}


uint32_t fat32_count_cluster_chain(int disk_no, uint32_t start_cluster) {
    uint32_t count = 0;
    uint32_t curr = start_cluster;

    while (is_valid_cluster(curr)) {
        count++;
        uint32_t next = fat32_get_next_cluster(disk_no, curr);
        if (is_end_of_cluster_chain(next)) {
            break;
        }
        curr = next;
    }

    return count;
}


bool fat32_append_cluster(int disk_no, uint32_t start_cluster, uint32_t *new_cluster) {
    uint32_t curr = start_cluster;

    while (1) {
        uint32_t next = fat32_get_next_cluster(disk_no, curr);
        if (is_end_of_cluster_chain(next)) {
            break;
        }
        curr = next;
    }

    if (!fat32_allocate_cluster(disk_no, new_cluster)) {
        return false;
    }

    fat32_set_next_cluster(disk_no, curr, *new_cluster);
    fat32_set_next_cluster(disk_no, *new_cluster, 0x0FFFFFFF);  // End of chain
    return true;
}


bool fat32_validate_cluster_chain(int disk_no, uint32_t start_cluster) {
    uint32_t curr = start_cluster;

    while (is_valid_cluster(curr)) {
        uint32_t next = fat32_get_next_cluster(disk_no, curr);
        if (next == 0) return false;
        if (is_end_of_cluster_chain(next)) return true;
        curr = next;
    }

    return false;
}


bool fat32_find_free_dir_entry(int disk_no, uint32_t dir_cluster, uint32_t *out_cluster, uint32_t *out_offset) {
    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *buf = (uint8_t *) malloc(cluster_size);
    if (!buf) return false;

    uint32_t curr = dir_cluster;

    while (is_valid_cluster(curr)) {
        if (!fat32_read_cluster(disk_no, curr, buf)) {
            free(buf);
            return false;
        }

        for (uint32_t offset = 0; offset < cluster_size; offset += 32) {
            uint8_t first = buf[offset];
            if (first == 0x00 || first == 0xE5) {
                *out_cluster = curr;
                *out_offset  = offset;
                free(buf);
                return true;
            }
        }

        uint32_t next = fat32_get_next_cluster(disk_no, curr);
        if (is_end_of_cluster_chain(next)) break;
        curr = next;
    }

    /* No free entry → extend directory */
    uint32_t new_cluster;
    if (!fat32_append_cluster(disk_no, dir_cluster, &new_cluster)) {
        free(buf);
        return false;
    }

    fat32_clear_cluster(disk_no, new_cluster);

    *out_cluster = new_cluster;
    *out_offset  = 0;

    free(buf);

    return true;
}



void fat32_format_83_name(const char *name, char out[11]) {
    memset(out, ' ', 11);

    int i = 0;  // Given string index
    int j = 0;  // Formatted string index

    // Name part (8 chars max)
    while (name[i] && name[i] != '.' && j < 8) {
        out[j++] = toupper((unsigned char)name[i++]);
    }

    // Skip to extension if dot exists
    while (name[i] && name[i] != '.') {
        i++;
    }

    // Extension part
    if (name[i] == '.') {
        i++;
        j = 8;
        while (name[i] && j < 11) {
            out[j++] = toupper((unsigned char)name[i++]);
        }
    }
}

bool fat32_create_dir_entry(int disk_no, uint32_t parent_cluster, const char *name, uint8_t attr, uint32_t first_cluster , uint32_t file_size) {
    uint32_t entry_cluster, entry_offset;

    if (!fat32_find_free_dir_entry( disk_no, parent_cluster, &entry_cluster, &entry_offset)) {
        return false;
    }

    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *buf = malloc(cluster_size);
    if (!buf) return false;

    if (!fat32_read_cluster(disk_no, entry_cluster, buf)) {
        free(buf);
        return false;
    }

    DirEntry *entry = (DirEntry *)(buf + entry_offset);

    memset(entry, 0, sizeof(DirEntry));
    fat32_format_83_name(name, entry->DIR_Name);

    entry->DIR_Attr = attr;
    entry->DIR_FstClusHI = (first_cluster >> 16) & 0xFFFF;
    entry->DIR_FstClusLO = first_cluster & 0xFFFF;
    entry->DIR_FileSize = file_size;

    bool ok = fat32_write_cluster(disk_no, entry_cluster, buf);
    free(buf);
    return ok;
}

bool fat32_init_directory(int disk_no, uint32_t dir_cluster, uint32_t parent_cluster)
{
    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *buf = malloc(cluster_size);
    if (!buf) return false;

    memset(buf, 0, cluster_size);

    DirEntry *dot = (DirEntry *)buf;

    DirEntry *dotdot = (DirEntry *)(buf + 32);

    /* "." entry */
    memset(dot, 0, sizeof(DirEntry));
    memset(dot->DIR_Name, ' ', 11);
    dot->DIR_Name[0] = '.';
    dot->DIR_Attr = ATTR_DIRECTORY;
    dot->DIR_FstClusLO = dir_cluster & 0xFFFF;
    dot->DIR_FstClusHI = dir_cluster >> 16;
    dot->DIR_FileSize = 0;

    /* ".." entry */
    memset(dotdot, 0, sizeof(DirEntry));
    memset(dotdot->DIR_Name, ' ', 11);
    dotdot->DIR_Name[0] = '.';
    dotdot->DIR_Name[1] = '.';
    dotdot->DIR_Attr = ATTR_DIRECTORY;
    dotdot->DIR_FstClusLO = parent_cluster & 0xFFFF;
    dotdot->DIR_FstClusHI = parent_cluster >> 16;
    dotdot->DIR_FileSize = 0;

    bool ok = fat32_write_cluster(disk_no, dir_cluster, buf);
    free(buf);
    return ok;
}

bool fat32_mkdir_internal(int disk_no, uint32_t parent_cluster, const char *name) {
    uint32_t new_cluster;

    if (!fat32_allocate_cluster(disk_no, &new_cluster))
        return false;

    fat32_set_next_cluster(disk_no, new_cluster, 0x0FFFFFFF);

    fat32_clear_cluster(disk_no, new_cluster);

    if (!fat32_init_directory(disk_no, new_cluster, parent_cluster))
        return false;

    return fat32_create_dir_entry( disk_no, parent_cluster, name, ATTR_DIRECTORY, new_cluster, 0 );
}

bool fat32_dir_exists(int disk_no, uint32_t dir_cluster, const char *name) {
    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *buf = malloc(cluster_size);
    if (!buf) return false;

    uint32_t curr = dir_cluster;
    char short_name[11];
    fat32_format_83_name(name, short_name);

    while (is_valid_cluster(curr)) {
        fat32_read_cluster(disk_no, curr, buf);

        for (uint32_t off = 0; off < cluster_size; off += 32) {
            DirEntry *e =
                (DirEntry *)(buf + off);

            if (e->DIR_Name[0] == 0x00) goto done;
            if (e->DIR_Name[0] == 0xE5) continue;
            if (e->DIR_Attr == ATTR_LONG_NAME) continue;

            if (memcmp(e->DIR_Name, short_name, 11) == 0) {
                free(buf);
                return true;
            }
        }

        uint32_t next = fat32_get_next_cluster(disk_no, curr);
        if (is_end_of_cluster_chain(next)) break;
        curr = next;
    }

    done:
        free(buf);

    return false;
}

bool fat32_mount(int disk_no, uint64_t partition_lba_start) {
    fat32_base_lba = partition_lba_start;

    uint8_t sector[512];

    if(!kebla_disk_read(disk_no, partition_lba_start, 1, sector)){
        printf("FAT32: boot sector read failed\n");
        return false;
    }

    if(!bpb){
        bpb = malloc(sizeof(BPB));
        if (!bpb) {
            printf("FAT32: BPB alloc failed\n");
            return false;
        }
    }
    
    memcpy(bpb, sector, sizeof(BPB));

    /* Validate FAT32 */
    if (bpb->BPB_BytsPerSec != 512) {
        printf("FAT32: invalid sector size\n");
        return false;
    }

    if (bpb->BPB_FATSz32 == 0) {
        printf("FAT32: not FAT32\n");
        return false;
    }

    if (bpb->BPB_NumFATs == 0) {
        printf("FAT32: invalid FAT count\n");
        return false;
    }

    if (bpb->BPB_SecPerClus == 0) {
        printf("FAT32: invalid cluster size\n");
        return false;
    }

    fat32_cwd_cluster = bpb->BPB_RootClus;


    // printf("FAT32 mounted\n");
    // printf("Bytes/sector: %u\n", bpb->BPB_BytsPerSec);
    // printf("Sectors/cluster: %u\n", bpb->BPB_SecPerClus);
    // printf("Reserved sectors: %u\n", bpb->BPB_RsvdSecCnt);
    // printf("FAT size: %u\n", bpb->BPB_FATSz32);
    // printf("Root cluster: %u\n", bpb->BPB_RootClus);
    // printf("Total clusters: %u\n", get_total_clusters());

    return true;
}

bool fat32_find_dir(int disk_no, uint32_t dir_cluster, const char *name, uint32_t *out_cluster)
{
    uint32_t cluster_size = bpb->BPB_BytsPerSec * bpb->BPB_SecPerClus;

    uint8_t *buf = malloc(cluster_size);
    if (!buf) return false;
    memset(buf, 0, cluster_size);

    if (!fat32_read_cluster_chain(disk_no, dir_cluster, buf, cluster_size)) {
        free(buf);
        printf("Reading Cluster Chain failed\n");
        return false;
    }

    char name83[12];
    fat32_format_83_name(name, name83);

    name83[11] = '\0';

    // printf("[FAT32_FIND_DIR] Directory name: %s is searching!\n", name83);

    DirEntry *entry = (DirEntry *)buf;
    uint32_t entries = cluster_size / sizeof(DirEntry);

    for (uint32_t i = 0; i < entries; i++) {
        if (entry[i].DIR_Name[0] == 0x00) break;
        if (entry[i].DIR_Name[0] == 0xE5) continue;

        if ((entry[i].DIR_Attr & ATTR_DIRECTORY) &&  memcmp(entry[i].DIR_Name, name83, 11) == 0)
        {
            *out_cluster =  (entry[i].DIR_FstClusHI << 16) |  entry[i].DIR_FstClusLO;

            free(buf);
            return true;
        }
    }

    free(buf);
    
    // printf("[FAT32_find_dir] No directory entry found with given name %s in given cluster %llu\n", name, dir_cluster);

    return false;
}



bool fat32_create_file_in_dir(int disk_no, uint32_t parent_cluster, const char *filename, const char *content, uint32_t size)
{
    uint32_t first_cluster = 0;

    if (!fat32_write_cluster_chain(disk_no, content, size, &first_cluster))
        return false;

    return fat32_create_dir_entry( disk_no, parent_cluster, filename, ATTR_ARCHIVE, first_cluster,  size);
}


bool fat32_change_current_directory(int disk_no, const char *path)
{
    if (!path || !path[0])
        return false;

    uint32_t current;

    /* absolute path */
    if (path[0] == '/')
        current = bpb->BPB_RootClus;
    else
        current = fat32_cwd_cluster;

    /* root */
    if (strcmp(path, "/") == 0) {
        fat32_cwd_cluster = bpb->BPB_RootClus;
        return true;
    }

    char tmp[256];
    strcpy(tmp, path);

    char *token = strtok(tmp, "/");

    while (token) {

        if (strcmp(token, ".") == 0) {
            /* do nothing */
        }
        else if (strcmp(token, "..") == 0) {
            /* read parent from ".." entry */
            uint32_t parent;
            if (fat32_find_dir(disk_no, current, "..", &parent))
                current = parent;
        }
        else {
            uint32_t next;
            if (!fat32_find_dir(disk_no, current, token, &next)) {
                printf("Directory not found: %s\n", token);
                return false;
            }
            current = next;
        }

        token = strtok(NULL, "/");
    }

    fat32_cwd_cluster = current;

    return true;
}

bool fat32_path_to_cluster(int disk_no, const char *path, uint32_t *out_cluster)
{
    if (!path || !out_cluster || !bpb)
        return false;

    char *path_copy = strdup(path);
    if (!path_copy)
        return false;

    uint32_t cluster;

    // Absolute path → start from root
    if (path[0] == '/') {
        cluster = bpb->BPB_RootClus;
    } else {
        cluster = fat32_cwd_cluster;
    }

    char *saveptr;

    char *token = strtok(path_copy, "/");

    while (token) {
        if (strcmp(token, ".") == 0) {
            token = strtok(NULL, "/");
            continue;
        }

        if (strcmp(token, "..") == 0) {
            uint32_t parent;
            if (!fat32_find_dir(disk_no, cluster, "..", &parent)) {
                free(path_copy);
                return false;
            }
            cluster = parent;
            token = strtok(NULL, "/");
            continue;
        }

        uint32_t next_cluster;
        if (!fat32_find_dir(disk_no, cluster, token, &next_cluster)) {
            free(path_copy);
            return false;
        }

        cluster = next_cluster;
        token = strtok(NULL, "/");
    }

    free(path_copy);

    *out_cluster = cluster;

    return true;
}

bool fat32_mkdir_root(int disk_no, const char *name) {
    uint32_t root = bpb->BPB_RootClus;

    if (fat32_dir_exists(disk_no, root, name)) {
        printf("Directory already exists\n");
        return false;
    }

    if (!fat32_mkdir_internal(disk_no, root, name)) {
        printf("mkdir failed\n");
        return false;
    }

    return true;
}

bool fat32_mkdir(int disk_no, const char* dirpath){
    if (!dirpath || !bpb) return false;

    char path_copy[256];
    strncpy(path_copy, dirpath, sizeof(path_copy));

    path_copy[sizeof(path_copy) - 1] = '\0';

    // Remove trailing '/'
    size_t len = strlen(path_copy);
    if (len > 1 && path_copy[len - 1] == '/'){
        path_copy[len - 1] = '\0';
    }

    char *last_slash = strrchr(path_copy, '/');

    uint32_t parent_cluster;
    char *dirname;

    if (!last_slash) {
        parent_cluster = fat32_cwd_cluster;
        dirname = path_copy;
    }
    else if (last_slash == path_copy) {
        // parent is root
        parent_cluster = bpb->BPB_RootClus;
        dirname = last_slash + 1;
    }
    else {
        *last_slash = '\0';
        dirname = last_slash + 1;

        if (!fat32_path_to_cluster(disk_no, path_copy, &parent_cluster))
            return false;
    }

    if (strlen(dirname) == 0)
        return false;

    // already exists?
    if (fat32_dir_exists(disk_no, parent_cluster, dirname))
        return false;

    return fat32_mkdir_internal(disk_no, parent_cluster, dirname);
        
}

bool fat32_find_file( int disk_no, uint32_t dir_cluster, const char *name, DirEntry *out_entry, uint32_t *entry_cluster, uint32_t *entry_offset)
{
    uint32_t cluster_size = bpb->BPB_BytsPerSec * bpb->BPB_SecPerClus;

    uint8_t *buf = malloc(cluster_size);
    if (!buf) return false;

    if (!fat32_read_cluster_chain(disk_no, dir_cluster, buf, cluster_size)) {
        free(buf);
        return false;
    }

    char name83[11];
    fat32_format_83_name(name, name83);

    DirEntry *entry = (DirEntry *)buf;
    uint32_t entries = cluster_size / sizeof(DirEntry);

    for (uint32_t i = 0; i < entries; i++) {
        if (entry[i].DIR_Name[0] == 0x00) break;
        if (entry[i].DIR_Name[0] == 0xE5) continue;

        if (!(entry[i].DIR_Attr & ATTR_DIRECTORY) &&
            memcmp(entry[i].DIR_Name, name83, 11) == 0)
        {
            memcpy(out_entry, &entry[i], sizeof(DirEntry));

            *entry_cluster = dir_cluster;
            *entry_offset = i;

            free(buf);
            return true;
        }
    }

    free(buf);

    return false;
}

bool fat32_open(int disk_no, const char *path, FAT32_FILE *file)
{
    if (!file || !path)
        return false;

    char tmp[256];
    strcpy(tmp, path);

    char *last = strrchr(tmp, '/');

    uint32_t parent_cluster;
    char *filename;

    if (!last) {
        parent_cluster = fat32_cwd_cluster;
        filename = tmp;
    } else if (last == tmp) {
        parent_cluster = bpb->BPB_RootClus;
        filename = last + 1;
    } else {
        *last = '\0';
        filename = last + 1;

        if (!fat32_path_to_cluster(disk_no, tmp, &parent_cluster)){
            return false;
        }  
    }

    DirEntry entry;
    uint32_t ec, eo;

    if (!fat32_find_file(disk_no, parent_cluster, filename, &entry, &ec, &eo))
        return false;

    file->first_cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;

    file->size = entry.DIR_FileSize;
    file->pos = 0;
    file->parent_cluster = parent_cluster;
    memcpy(file->name, entry.DIR_Name, 11);

    return true;
}

uint32_t fat32_read(int disk_no, FAT32_FILE *file, void *buffer, uint32_t size)
{
    if(!file || !buffer){
        return 0;
    }

    if(file->first_cluster == 0){
        printf("file->first_cluster:%d\n", file->first_cluster);
        return 0;
    }
        

    if(file->pos >= file->size){
        printf("file->pos: %d, file->size: %d\n", file->pos, file->size);
        return 0;
    }
       

    uint32_t remaining = file->size - file->pos;
    if(size > remaining){
        size = remaining;
    }
        
    if(!fat32_read_cluster_chain( disk_no, file->first_cluster, buffer, size)){
        return 0;
    }
        
    file->pos += size;

    return size;
}



uint32_t fat32_write(int disk_no, FAT32_FILE *file, const void *buffer, uint32_t size)
{
    if (!file || !buffer)
        return 0;

    fat32_free_cluster_chain(disk_no, file->first_cluster);

    uint32_t new_cluster;

    if (!fat32_write_cluster_chain( disk_no, buffer, size,  &new_cluster))
        return 0;

    file->first_cluster = new_cluster;
    file->size = size;
    file->pos = size;

    return size;
}





// Testing 8.3 Filename FAT32 Test
bool fat32_test(int disk_no, uint64_t fat_base_lba){

    if(!fat32_mount(disk_no, fat_base_lba)){
        printf("[FAT32 TEST] Failed to Mount FAT32 FS at LBA: %d!\n", fat_base_lba);
        return false;
    }
    printf("[FAT32 TEST] Successfully Mount Disk %d.\n", disk_no);
    

    uint32_t root_cluster = bpb->BPB_RootClus;
    
    // Crating a directory at root
    char *dir_path = "TESTDIR";
    if(!fat32_mkdir(disk_no, dir_path)){
        printf("[FAT32 TEST] Creating Directory %s is failed!\n", dir_path);
        return false;
    }
    printf("[FAT32 TEST] Creating Directory %s is success.\n", dir_path);

    // Finding Directory Cluster no
    uint32_t dir_cluster_no = 0;
    if(!fat32_path_to_cluster(disk_no, dir_path, &dir_cluster_no)){
        printf("[FAT32 TEST] Failed to get Cluster no for %s", dir_path);
        return false;
    }
    printf("[FAT32 TEST] Successfully get Cluster no %d for directory %s\n", dir_cluster_no, dir_path);

    // Creating testfile.text
    char *file_name = "TESTFILE.TXT";   // 8.3 Short Filename
    char *buff = "This is a test text string for testing fat32 filesystem.";
    uint32_t file_size = strlen(buff);

    if(!fat32_create_file_in_dir(disk_no, dir_cluster_no, file_name, buff, file_size)){
        return false;
    }
    printf("[FAT32 TEST] Successfully created %s\n\n", file_name);

    // Opening testfile.txt
    const char *file_path = "TESTDIR/TESTFILE.TXT";
    FAT32_FILE file;
    if(!fat32_open(disk_no, file_path, &file)){
        printf("[FAT32 TEST] Faile to read file %s\n", file_path);
        return false;
    }
    printf("[FAT32 TEST] Successfully open file %s\n", file_path);

    // Reading testfile.txt 
    char *buffer = (char *) malloc(file_size);
    uint32_t rb = fat32_read(disk_no, &file, buffer, file_size);
    if(rb <= 0){
        printf("[FAT32 TEST] Failed to read file %s!\n", file_path);
        // return false;
    }
    printf("[FAT32 TEST] Successfully read %d bytes\n", rb);
    printf("[FAT32 TEST] File content: %s\n", buffer);

    free(buffer);

    return true;
}




// LFN

uint8_t fat32_lfn_checksum(const uint8_t short_name[11])
{
    uint8_t sum = 0;
    for (int i = 0; i < 11; i++)
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short_name[i];
    return sum;
}



