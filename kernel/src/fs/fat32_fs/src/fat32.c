
#include "../../../lib/string.h"
#include "../../../lib/stdlib.h"
#include "../../../lib/stdio.h"
#include "../../../lib/ctype.h"

#include "../include/fat32_bpb.h"
#include "../include/fat32.h"

extern uint32_t fat32_cwd_cluster;          // Defined in cluster_manager
extern BPB *bpb;                            // Defined in fat32


// This function changes the current working directory to the specified path, which can be either absolute or relative.
bool fat32_change_current_directory( const char *path)
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
            if (fat32_find_dir( current, "..", &parent))
                current = parent;
        }
        else {
            uint32_t next;
            if (!fat32_find_dir( current, token, &next)) {
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



bool fat32_mkdir( const char* dirpath) {
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

        if (!fat32_path_to_cluster( path_copy, &parent_cluster)){
                printf("Parent directory not found: %s\n", path_copy);
                return false;
        }
    }

    if (strlen(dirname) == 0){
        printf("Invalid directory name\n");
        return false;
    }
        

    // already exists?
    if (fat32_dir_exists( parent_cluster, dirname)){
        printf("Directory already exists: %s\n", dirname);
        return false;
    }
        

    bool ok = fat32_mkdir_internal( parent_cluster, dirname);
    if (!ok) {
        printf("Failed to create directory: %s\n", dirname);
        return false;
    }
    return true;
        
}


bool fat32_open( const char *path, FAT32_FILE *file)
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

        if (!fat32_path_to_cluster( tmp, &parent_cluster)){
            return false;
        }  
    }

    DirEntry entry;
    uint32_t ec, eo;

    if (!fat32_find_file( parent_cluster, filename, &entry, &ec, &eo))
        return false;

    file->first_cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;

    file->size = entry.DIR_FileSize;
    file->pos = 0;
    file->parent_cluster = parent_cluster;
    memcpy(file->name, entry.DIR_Name, 11);

    return true;
}

uint32_t fat32_read(FAT32_FILE *file, void *buffer, uint32_t size)
{
    if (!file || !buffer)
        return 0;

    if (file->pos >= file->size)
        return 0;

    uint32_t remaining = file->size - file->pos;
    if (size > remaining)
        size = remaining;

    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *cluster_buf = malloc(cluster_size);
    if (!cluster_buf)
        return 0;

    uint32_t current = file->first_cluster;
    uint32_t bytes_read = 0;
    uint32_t file_offset = file->pos;

    // Skip clusters until reaching correct offset
    while (file_offset >= cluster_size && is_valid_cluster(current)) {
        file_offset -= cluster_size;
        current = fat32_get_next_cluster(current);
    }

    while (bytes_read < size && is_valid_cluster(current)) {

        if (!fat32_read_cluster(current, cluster_buf)) {
            free(cluster_buf);
            return 0;
        }

        uint32_t copy_start = file_offset;
        uint32_t copy_bytes = cluster_size - copy_start;

        if (copy_bytes > (size - bytes_read))
            copy_bytes = size - bytes_read;

        memcpy(
            (uint8_t*)buffer + bytes_read,
            cluster_buf + copy_start,
            copy_bytes
        );

        bytes_read += copy_bytes;
        file_offset = 0;
        current = fat32_get_next_cluster(current);
    }

    file->pos += bytes_read;
    free(cluster_buf);

    return bytes_read;
}



uint32_t fat32_write( FAT32_FILE *file, const void *buffer, uint32_t size)
{
    if (!file || !buffer)
        return 0;

    fat32_free_cluster_chain( file->first_cluster);

    uint32_t new_cluster;

    if (!fat32_write_cluster_chain(  buffer, size,  &new_cluster))
        return 0;

    file->first_cluster = new_cluster;
    file->size = size;
    file->pos = size;

    return size;
}





