#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../include/fat32_types.h"
#include "../include/fat32_bpb.h"
#include "../include/fat32_fsinfo.h"
#include "../include/fat32_mount.h"
#include "../include/fat32_utility.h"
#include "../include/lfn.h"

#include "../include/cluster_manager.h"



#define SECTOR_SIZE 512

extern uint64_t fat32_base_lba;             // Defined in fat32_mount.c

uint32_t fat32_cwd_cluster = 2;             // Current Working Directory Cluster

uint32_t fat32_free_cluster_no = 2;         // Hint for next free cluster (starts from 2 as 0 and 1 are reserved)




// --------------------------- Cluster Management Functions ---------------------------

// This function writes zeros to all sectors of a given cluster
bool fat32_zero_cluster( uint32_t cluster_no)
{

    uint32_t first_sector = get_first_sector_of_cluster(cluster_no); // first sector of the given cluster

    uint32_t bytes = get_sectors_per_cluster() * get_bytes_per_sector();

    uint8_t *zero = malloc(bytes);
    if (!zero) return false;

    memset(zero, 0, bytes);

    bool ok = fat32_write_sectors( first_sector, get_sectors_per_cluster(), zero); // Clear all sectors 

    free(zero);

    return ok;
}

// read a single cluster and store it in given buffer.
bool fat32_read_cluster( uint32_t cluster_number, void *buffer){
    uint32_t first_sector = get_first_sector_of_cluster(cluster_number);
    uint8_t *buf_ptr = (uint8_t *)buffer;
    
    for(uint8_t i = 0; i < get_sectors_per_cluster(); i++){
        if(!fat32_read_sector(first_sector + i, buf_ptr + (i * get_bytes_per_sector()))){
            return false;
        }
    }
    return true;
}

// write a single cluster from given buffer
 bool fat32_write_cluster( uint32_t cluster_number, const void *buffer)
{
    uint32_t first_sector = get_first_sector_of_cluster(cluster_number);

    uint8_t sector_per_cluster = get_sectors_per_cluster();

    return fat32_write_sectors( first_sector, sector_per_cluster, buffer );
}

// Clearing a single cluster
 bool fat32_clear_cluster( uint32_t cluster) {
    uint32_t cluster_size = get_bytes_per_sector() * get_sectors_per_cluster();
    uint8_t *zero = malloc(cluster_size);
    if (!zero) return false;

    memset(zero, 0, cluster_size);
    bool ok = fat32_write_cluster( cluster, zero);
    free(zero);
    return ok;
}

 uint32_t fat32_get_next_cluster( uint32_t current_cluster){

    uint32_t fat_offset = current_cluster * 4; // Total bytes as Each FAT32 entry is 4 bytes
    
    uint32_t fat_sector_number = get_first_fat_sector() + (fat_offset / get_bytes_per_sector());
    uint32_t ent_offset = fat_offset % get_bytes_per_sector(); 

    uint8_t sector_buffer[get_bytes_per_sector()];
    if (!fat32_read_sector( fat_sector_number, sector_buffer)) {
        return 0;               // Error reading sector
    }

    uint32_t next_cluster = *(uint32_t *)&sector_buffer[ent_offset];    // As Current Cluster's FAT wrote next Cluster number
    next_cluster &= 0x0FFFFFFF; // Mask to get the lower 28 bits

    return next_cluster;
}

 bool fat32_set_next_cluster(uint32_t current_cluster, uint32_t next_cluster)
{
    if (!is_valid_cluster(current_cluster))
        return false;

    uint32_t bytes_per_sector = get_bytes_per_sector();

    uint32_t fat_offset = current_cluster * 4;

    uint32_t fat_sector_number = get_first_fat_sector() + (fat_offset / bytes_per_sector);

    uint32_t ent_offset = fat_offset % bytes_per_sector;

    uint8_t sector_buffer[512];

    // Read FAT sector
    if (!fat32_read_sector(fat_sector_number, sector_buffer))
        return false;

    // Update FAT entry
    uint32_t *entry = (uint32_t *)(sector_buffer + ent_offset);

    *entry = (*entry & 0xF0000000) | (next_cluster & 0x0FFFFFFF);

    // Write to all FAT copies
    for (uint8_t i = 0; i < get_total_no_fat(); i++)
    {
        uint32_t target_sector = fat_sector_number + (i * get_fat_size_in_sectors());

        if (!fat32_write_sector(target_sector, sector_buffer))
            return false;
    }

    return true;
}




// --------------------------Cluster Chain Management Functions--------------------------
 bool fat32_validate_cluster_chain( uint32_t start_cluster) {
    uint32_t curr = start_cluster;

    while (is_valid_cluster(curr)) {
        uint32_t next = fat32_get_next_cluster( curr);
        if (next == 0) return false;
        if (is_end_of_cluster_chain(next)) return true;
        curr = next;
    }

    return false;
}

 bool fat32_free_cluster_chain( uint32_t start_cluster){
    uint32_t current_cluster = start_cluster;

    while (is_valid_cluster(current_cluster)) {
        uint32_t next_cluster = fat32_get_next_cluster( current_cluster);
        if (next_cluster == 0) {
            return false; // Error reading next cluster
        }

        // Mark current cluster as free
        if (!fat32_set_next_cluster( current_cluster, 0x00000000)) {
            return false; // Error setting cluster
        }

        if (is_end_of_cluster_chain(next_cluster)) {
            break; // Reached end of chain
        }

        current_cluster = next_cluster;
    }

    return true;
}



// Allocate a free cluster and return its number
 bool fat32_allocate_cluster( uint32_t *allocated_cluster)
{
    uint32_t total_clusters = get_total_clusters();

    for (uint32_t cluster = fat32_free_cluster_no; cluster < total_clusters + 1; cluster++)
    {
        uint32_t fat_offset = cluster * 4;  // Each entry is 4 bytes
        uint32_t fat_sector_number = get_first_fat_sector() + (fat_offset / get_bytes_per_sector());
        uint32_t ent_offset =  fat_offset % get_bytes_per_sector();

        uint8_t sector_buffer[get_bytes_per_sector()];

        if (!fat32_read_sector( fat_sector_number, sector_buffer))
            return false;

        uint32_t entry = *(uint32_t *)&sector_buffer[ent_offset] & 0x0FFFFFFF;

        if (entry == 0) {                                             // Found a free cluster
            fat32_set_next_cluster( cluster, CLUSTER_END_OF_CHAIN);   // Mark it as end of chain
            *allocated_cluster = cluster;

            fat32_free_cluster_no = cluster + 1;                      // update fat32_free_cluster_no

            return true;
        }
    }

    return false;
}

 bool fat32_allocate_cluster_chain( uint32_t count, uint32_t *first_cluster){
    uint32_t prev_cluster = 0;
    *first_cluster = 0;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t new_cluster;
        if (!fat32_allocate_cluster( &new_cluster)) {
            return false; // Allocation failed
        }

        if (prev_cluster != 0) {
            if (!fat32_set_next_cluster( prev_cluster, new_cluster)) {
                return false; // Error linking clusters
            }
        } else {
            *first_cluster = new_cluster; // Set first cluster
        }

        prev_cluster = new_cluster;
    }

    // Mark the last cluster as end of chain
    if (!fat32_set_next_cluster( prev_cluster, CLUSTER_END_OF_CHAIN)) {
        return false; // Error setting end of chain
    }

    return true;
}

// reading cluster chain from given cluster
bool fat32_read_cluster_chain( uint32_t start_cluster, void *buffer, uint32_t max_bytes) {

    if(start_cluster < get_root_dir_cluster() || !buffer){
        printf("fat32_read_cluster_chain: Invalid Cluster No %d or Invalid Buffer\n", start_cluster);
    }

    uint8_t *buf = (uint8_t *)buffer;
    uint32_t current = start_cluster;
    uint32_t bytes_read = 0;
    uint32_t cluster_size = get_cluster_size_bytes();

    while (is_valid_cluster(current)) {

        printf("fat32_read_cluster_chain: %d\n", current);

        if (bytes_read + cluster_size > max_bytes) {
            return false; // Buffer too small
        }

        if (!fat32_read_cluster( current, buf + bytes_read)) {
            return false;   // If reading fails, return false
        }

        bytes_read += cluster_size;

        uint32_t next = fat32_get_next_cluster( current);
        if (is_end_of_cluster_chain(next)) {
            break;
        }

        current = next;
    }

    return true;
}

// writing cluster chain starting from given cluster
 bool fat32_write_cluster_chain( const void *buffer, uint32_t size, uint32_t *first_cluster)
{
    if(!buffer ) return false;
    const uint8_t *buf = (const uint8_t *)buffer;
    uint32_t cluster_size = get_cluster_size_bytes();
    uint32_t written = 0;

    uint32_t prev_cluster = 0;
    uint32_t curr_cluster = 0;

    uint8_t *temp = malloc(cluster_size);
    if (!temp) return false;


    while (written < size) {
        // 1. Allocate a new cluster
        if (!fat32_allocate_cluster( &curr_cluster)) {
            free(temp);
            return false;
        }

        // 2. Link it to the previous cluster in the chain
        if (prev_cluster != 0) {
            fat32_set_next_cluster( prev_cluster, curr_cluster);
        } else {
            *first_cluster = curr_cluster;
        }

        uint32_t to_write =  (size - written > cluster_size)  ? cluster_size : (size - written);

        // 3. Write data to the current cluster
        if (to_write == cluster_size) {
            if(!fat32_write_cluster( curr_cluster, buf + written)){
                free(temp);
                return false;
            }
        } else {
            memset(temp, 0, cluster_size);
            memcpy(temp, buf + written, to_write);
            if(!fat32_write_cluster( curr_cluster, temp)){
                free(temp);
                return false;
            }
        }

        written += to_write;

        prev_cluster = curr_cluster;
    }


    // 4. Mark the end of the cluster chain
    fat32_set_next_cluster( prev_cluster, CLUSTER_END_OF_CHAIN);

    free(temp);

    return true;
}


// Count the number of clusters in a chain starting from given cluster
 uint32_t fat32_count_cluster_chain( uint32_t start_cluster) {
    uint32_t count = 0;
    uint32_t curr = start_cluster;

    while (is_valid_cluster(curr)) {
        count++;
        uint32_t next = fat32_get_next_cluster( curr);
        if (is_end_of_cluster_chain(next)) {
            break;
        }
        curr = next;
    }

    return count;
}

 bool fat32_append_cluster( uint32_t start_cluster, uint32_t *new_cluster) {
    uint32_t curr = start_cluster;

    while (1) {
        uint32_t next = fat32_get_next_cluster( curr);
        if (is_end_of_cluster_chain(next)) {
            break;
        }
        curr = next;
    }

    if (!fat32_allocate_cluster( new_cluster)) {
        return false;
    }

    fat32_set_next_cluster( curr, *new_cluster);
    fat32_set_next_cluster( *new_cluster, CLUSTER_END_OF_CHAIN);  // End of chain
    return true;
}

bool fat32_find_contiguous_free_entries(uint32_t dir_cluster, uint32_t needed, uint32_t *out_cluster,  uint32_t *out_offset)
{
    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *buf = malloc(cluster_size);
    if (!buf) return false;

    uint32_t curr = dir_cluster;

    while (is_valid_cluster(curr)) {

        if (!fat32_read_cluster(curr, buf)) {
            free(buf);
            return false;
        }

        uint32_t free_count = 0;

        for (uint32_t off = 0; off < cluster_size; off += 32) {

            uint8_t first = buf[off];

            if (first == 0x00 || first == 0xE5) {
                free_count++;
                if (free_count == needed) {
                    *out_cluster = curr;
                    *out_offset = off - (needed - 1) * 32;
                    free(buf);
                    return true;
                }
            } else {
                free_count = 0;
            }
        }

        uint32_t next = fat32_get_next_cluster(curr);
        if (is_end_of_cluster_chain(next))
            break;

        curr = next;
    }

    // No space → extend directory
    uint32_t new_cluster;
    if (!fat32_append_cluster(dir_cluster, &new_cluster)) {
        free(buf);
        return false;
    }

    fat32_clear_cluster(new_cluster);

    *out_cluster = new_cluster;
    *out_offset = 0;

    free(buf);
    return true;
}

// This function searches for a free directory entry in the specified directory cluster.
 bool fat32_find_free_dir_entry( uint32_t dir_cluster, uint32_t *out_cluster, uint32_t *out_offset) {
    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *buf = (uint8_t *) malloc(cluster_size);
    if (!buf) return false;

    uint32_t curr = dir_cluster;

    while (is_valid_cluster(curr)) {
        if (!fat32_read_cluster( curr, buf)) {
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

        uint32_t next = fat32_get_next_cluster( curr);
        if (is_end_of_cluster_chain(next)) break;
        curr = next;
    }

    /* No free entry → extend directory */
    uint32_t new_cluster;
    if (!fat32_append_cluster( dir_cluster, &new_cluster)) {
        free(buf);
        return false;
    }

    fat32_clear_cluster( new_cluster);

    *out_cluster = new_cluster;
    *out_offset  = 0;

    free(buf);

    return true;
}


// This function converts a given filename into the 8.3 format used in FAT32 directory entries.
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





// -------------------------- Directory Entry Management Functions -------------------------

// This function creates a directory entry in the specified parent directory cluster with the given name, attributes, starting cluster, and file size.
bool fat32_create_dir_entry(uint32_t parent_cluster, const char *name, uint8_t attr,  uint32_t first_cluster,  uint32_t file_size)
{
    uint32_t cluster_size = get_cluster_size_bytes();
    char short_name[11];
    fat32_format_83_name(name, short_name);

    uint8_t checksum = fat32_lfn_checksum((uint8_t*)short_name);

    int name_len = strlen(name);
    int lfn_count = fat32_needs_lfn(name) ? (name_len + 12) / 13 : 0;

    uint32_t total_entries = lfn_count + 1;

    uint32_t entry_cluster, entry_offset;

    if (!fat32_find_contiguous_free_entries(parent_cluster,  total_entries, &entry_cluster,  &entry_offset))
        return false;

    uint8_t *buf = malloc(cluster_size);
    if (!buf) return false;

    if (!fat32_read_cluster(entry_cluster, buf)) {
        free(buf);
        return false;
    }

    // ---------------- Write LFN entries ----------------
    for (int i = 0; i < lfn_count; i++) {

        LFNEntry *lfn =
            (LFNEntry *)(buf + entry_offset + (i * 32));

        memset(lfn, 0, sizeof(LFNEntry));

        int order = lfn_count - i;
        lfn->LDIR_Ord = order;
        if (order == lfn_count)
            lfn->LDIR_Ord |= 0x40;

        lfn->LDIR_Attr = ATTR_LONG_NAME;
        lfn->LDIR_Type = 0;
        lfn->LDIR_Chksum = checksum;
        lfn->LDIR_FstClusLO = 0;

        int start = (order - 1) * 13;

        for (int j = 0; j < 13; j++) {

            int idx = start + j;
            uint16_t ch;

            if (idx < name_len)
                ch = (uint16_t)name[idx];
            else if (idx == name_len)
                ch = 0x0000;
            else
                ch = 0xFFFF;

            if (j < 5)
                lfn->LDIR_Name1[j] = ch;
            else if (j < 11)
                lfn->LDIR_Name2[j - 5] = ch;
            else
                lfn->LDIR_Name3[j - 11] = ch;
        }
    }

    // ---------------- Write short entry ----------------
    DirEntry *entry = (DirEntry *)(buf + entry_offset + (lfn_count * 32));

    memset(entry, 0, sizeof(DirEntry));

    memcpy(entry->DIR_Name, short_name, 11);
    entry->DIR_Attr = attr;
    entry->DIR_FstClusHI = (first_cluster >> 16) & 0xFFFF;
    entry->DIR_FstClusLO = first_cluster & 0xFFFF;
    entry->DIR_FileSize = file_size;

    bool ok = fat32_write_cluster(entry_cluster, buf);

    free(buf);

    return ok;
}

// This function initializes a new directory cluster by creating the "." and ".." entries.
 bool fat32_init_directory( uint32_t dir_cluster, uint32_t parent_cluster)
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

    bool ok = fat32_write_cluster( dir_cluster, buf);
    free(buf);
    return ok;
}

// This function creates a new directory with the specified name under the given parent directory cluster.
 bool fat32_mkdir_internal( uint32_t parent_cluster, const char *name) {
    uint32_t new_cluster;

    if (!fat32_allocate_cluster( &new_cluster))
        return false;

    fat32_clear_cluster( new_cluster);

    if (!fat32_init_directory( new_cluster, parent_cluster))
        return false;

    return fat32_create_dir_entry( parent_cluster, name, ATTR_DIRECTORY, new_cluster, 0 );
}

 bool fat32_dir_exists(uint32_t dir_cluster, const char *name)
{
    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *buf = malloc(cluster_size);
    if (!buf) return false;

    uint32_t curr = dir_cluster;

    char short_name[11];
    fat32_format_83_name(name, short_name);

    char long_name[LONG_FILENAME_SIZE];
    memset(long_name, 0, sizeof(long_name));

    while (is_valid_cluster(curr)) {

        if (!fat32_read_cluster(curr, buf)) {
            free(buf);
            return false;
        }

        for (uint32_t off = 0; off < cluster_size; off += 32) {

            DirEntry *e = (DirEntry *)(buf + off);

            // End of directory
            if (e->DIR_Name[0] == 0x00)
                goto done;

            // Deleted entry
            if (e->DIR_Name[0] == 0xE5) {
                memset(long_name, 0, sizeof(long_name));
                continue;
            }

            // LFN entry
            if (e->DIR_Attr == ATTR_LONG_NAME) {

                LFNEntry *lfn = (LFNEntry *)e;

                int order = (lfn->LDIR_Ord & 0x1F) - 1;
                int pos = order * 13;

                for (int i = 0; i < 5; i++)
                    if (lfn->LDIR_Name1[i] != 0xFFFF && lfn->LDIR_Name1[i] != 0x0000)
                        long_name[pos++] = (char)lfn->LDIR_Name1[i];

                for (int i = 0; i < 6; i++)
                    if (lfn->LDIR_Name2[i] != 0xFFFF && lfn->LDIR_Name2[i] != 0x0000)
                        long_name[pos++] = (char)lfn->LDIR_Name2[i];

                for (int i = 0; i < 2; i++)
                    if (lfn->LDIR_Name3[i] != 0xFFFF && lfn->LDIR_Name3[i] != 0x0000)
                        long_name[pos++] = (char)lfn->LDIR_Name3[i];

                continue;
            }

            // Normal entry (directory only)
            if (e->DIR_Attr & ATTR_DIRECTORY) {

                // Compare LFN first
                if (long_name[0] != '\0') {
                    if (strcmp(long_name, name) == 0) {
                        free(buf);
                        return true;
                    }
                }

                // Compare 8.3
                if (memcmp(e->DIR_Name, short_name, 11) == 0) {
                    free(buf);
                    return true;
                }
            }

            // Reset LFN buffer after normal entry
            memset(long_name, 0, sizeof(long_name));
        }

        uint32_t next = fat32_get_next_cluster(curr);
        if (is_end_of_cluster_chain(next))
            break;

        curr = next;
    }

done:
    free(buf);
    return false;
}



// This function searches for a directory entry with the specified name in the given directory cluster and returns its starting cluster if found.
bool fat32_find_dir(uint32_t dir_cluster, const char *name, uint32_t *out_cluster)
{
    if (dir_cluster < get_root_dir_cluster() ||
        dir_cluster > get_total_clusters() + 2) {
        printf("fat32_find_dir: Invalid Cluster %u\n", dir_cluster);
        return false;
    }

    uint32_t cluster_size = get_bytes_per_sector() * get_sectors_per_cluster();

    uint8_t *buf = malloc(cluster_size);
    if (!buf)
        return false;

    char name83[12];
    fat32_format_83_name(name, name83);
    name83[11] = '\0';

    // printf("[FAT32_FIND_DIR] Directory name: %s is searching!\n", name83);

    uint32_t curr = dir_cluster;

    while (is_valid_cluster(curr)) {

        /* Read one cluster */
        if (!fat32_read_cluster(curr, buf)) {
            free(buf);
            return false;
        }

        DirEntry *entry = (DirEntry *)buf;
        uint32_t entries = cluster_size / sizeof(DirEntry);

        for (uint32_t i = 0; i < entries; i++) {

            /* End of directory */
            if (entry[i].DIR_Name[0] == 0x00) {
                free(buf);
                return false;
            }

            /* Deleted entry */
            if (entry[i].DIR_Name[0] == 0xE5)
                continue;

            /* Match directory */
            if ((entry[i].DIR_Attr & ATTR_DIRECTORY) &&
                memcmp(entry[i].DIR_Name, name83, 11) == 0)
            {
                *out_cluster =
                    ((uint32_t)entry[i].DIR_FstClusHI << 16) |
                     entry[i].DIR_FstClusLO;

                free(buf);
                return true;
            }
        }

        /* Move to next cluster in chain */
        uint32_t next = fat32_get_next_cluster(curr);

        if (is_end_of_cluster_chain(next))
            break;

        curr = next;
    }

    free(buf);
    return false;
}

// ---------------------------- File Management Functions ----------------------------
// This function creates a new file with the specified name and content under the given parent directory cluster.
 bool fat32_create_file_in_dir( uint32_t parent_cluster, const char *filename, const char *content, uint32_t size)
{
    uint32_t first_cluster = 0;

    if (!fat32_write_cluster_chain( content, size, &first_cluster))
        return false;

    return fat32_create_dir_entry( parent_cluster, filename, ATTR_ARCHIVE, first_cluster,  size);
}




bool fat32_path_to_cluster( const char *path, uint32_t *out_cluster)
{
    

    if (!path || !out_cluster || !bpb)
        return false;

    char *path_copy = (char *) strdup(path);
    if (!path_copy) return false;

    uint32_t cluster;

    // Absolute path → start from root
    if (path[0] == '/') {
        cluster = get_root_dir_cluster();
    } else {
        cluster = fat32_cwd_cluster;
    }

    char *token = strtok(path_copy, "/");

    while (token) {
        if (strcmp(token, ".") == 0) {
            token = strtok(NULL, "/");
            continue;
        }

        if (strcmp(token, "..") == 0) {
            uint32_t parent;
            if (!fat32_find_dir( cluster, "..", &parent)) {
                printf("Failed to get %s..\n", token);
                free(path_copy);
                return false;
            }
            cluster = parent;
            token = strtok(NULL, "/");
            continue;
        }

        uint32_t next_cluster;
        if (!fat32_find_dir( cluster, token, &next_cluster)) {
            printf("Failed to find %s\n");
            free(path_copy);
            return false;
        }

        cluster = next_cluster;
        token = strtok(NULL, "/");
    }

    free(path_copy);

    *out_cluster = cluster;

    // printf("fat32_path_to_cluster: Path %s to Cluster %d\n", path, *out_cluster);

    return true;
}





 bool fat32_find_file(uint32_t dir_cluster,  const char *name, DirEntry *out_entry,  uint32_t *entry_cluster, uint32_t *entry_offset)
{
    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *buf = malloc(cluster_size);
    if (!buf) return false;

    uint32_t curr = dir_cluster;
    char long_name[LONG_FILENAME_SIZE];
    memset(long_name, 0, sizeof(long_name));

    char short_name[11];
    fat32_format_83_name(name, short_name);

    while (is_valid_cluster(curr)) {

        fat32_read_cluster(curr, buf);

        for (uint32_t off = 0; off < cluster_size; off += 32) {

            DirEntry *e = (DirEntry *)(buf + off);

            if (e->DIR_Name[0] == 0x00)
                break;

            if (e->DIR_Name[0] == 0xE5)
                continue;

            if (e->DIR_Attr == ATTR_LONG_NAME) {

                LFNEntry *lfn = (LFNEntry *)e;

                int order = (lfn->LDIR_Ord & 0x1F) - 1;
                int pos = order * 13;

                for (int i = 0; i < 5; i++)
                    long_name[pos++] = (char)lfn->LDIR_Name1[i];

                for (int i = 0; i < 6; i++)
                    long_name[pos++] = (char)lfn->LDIR_Name2[i];

                for (int i = 0; i < 2; i++)
                    long_name[pos++] = (char)lfn->LDIR_Name3[i];

                continue;
            }

            if (strcmp(long_name, name) == 0 ||
                memcmp(e->DIR_Name, short_name, 11) == 0)
            {
                memcpy(out_entry, e, sizeof(DirEntry));
                *entry_cluster = curr;
                *entry_offset = off;
                free(buf);
                return true;
            }

            memset(long_name, 0, sizeof(long_name));
        }

        uint32_t next = fat32_get_next_cluster(curr);
        if (is_end_of_cluster_chain(next))
            break;

        curr = next;
    }

    free(buf);
    return false;
}



bool fat32_split_path(const char *path, uint32_t *parent_cluster, char *name)
{
    if (!path || !parent_cluster || !name)
        return false;

    char tmp[256];
    strcpy(tmp, path);

    char *last = strrchr(tmp, '/');

    if (!last) {
        *parent_cluster = fat32_cwd_cluster;
        strcpy(name, tmp);
    }
    else if (last == tmp) {
        *parent_cluster = get_root_dir_cluster();
        strcpy(name, last + 1);
    }
    else {
        *last = '\0';
        strcpy(name, last + 1);

        if (!fat32_path_to_cluster(tmp, parent_cluster))
            return false;
    }

    return true;
}



bool fat32_mkdir_root( const char *name) {
    uint32_t root = get_root_dir_cluster();

    if (fat32_dir_exists( root, name)) {
        printf("Directory already exists\n");
        return false;
    }

    if (!fat32_mkdir_internal( root, name)) {
        printf("mkdir failed\n");
        return false;
    }

    return true;
}


