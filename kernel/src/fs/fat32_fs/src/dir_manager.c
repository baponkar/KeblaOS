
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "../include/fat32_bpb.h"
#include "../include/fat32.h"
#include "../include/cluster_manager.h"

#include "../include/dir_manager.h"

extern uint32_t fat32_cwd_cluster;
char fat32_cwd_path[256];



bool f_cwd(char *path){

    if (!path || !path[0])  return false;

    uint32_t current;

    /* absolute path */
    if (path[0] == '/'){
        current = get_root_dir_cluster();
    }else{
        current = fat32_cwd_cluster;
    }

    /* root */
    if (strcmp(path, "/") == 0) {
        fat32_cwd_cluster = get_root_dir_cluster();
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

bool f_opendir(FAT32_DIR *dp, char *path)
{
    if (!dp || !path)
        return false;

    uint32_t cluster;

    /* root directory */
    if (strcmp(path, "/") == 0) {
        cluster = get_root_dir_cluster();
    }
    else {
        if (!fat32_path_to_cluster(path, &cluster))
            return false;
    }

    /* initialize directory handle */
    dp->first_cluster = cluster;
    dp->current_cluster = cluster;
    dp->pos = 0;

    /* determine parent cluster */
    if (path[0] == '/') {
        char tmp[256];
        strcpy(tmp, path);

        char *last = strrchr(tmp, '/');

        if (!last || last == tmp) {
            dp->parent_cluster = get_root_dir_cluster();
        }
        else {
            *last = '\0';
            fat32_path_to_cluster(tmp, &dp->parent_cluster);
        }
    }
    else {
        dp->parent_cluster = fat32_cwd_cluster;
    }

    strncpy(dp->name, path, sizeof(dp->name) - 1);
    dp->name[sizeof(dp->name) - 1] = '\0';

    return true;
}


bool f_closedir(FAT32_DIR *dp)
{
    if (!dp)
        return false;

    dp->first_cluster = 0;
    dp->current_cluster = 0;
    dp->pos = 0;
    dp->parent_cluster = 0;

    dp->name[0] = '\0';

    return true;
}


bool f_readdir(FAT32_DIR *dp, FAT32_DIRENT *entry)
{
    if (!dp || !entry)   return false;

    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *cluster_buf = malloc(cluster_size);

    if (!cluster_buf)
        return false;

    while (is_valid_cluster(dp->current_cluster))
    {
        if (!fat32_read_cluster(dp->current_cluster, cluster_buf))
        {
            free(cluster_buf);
            return false;
        }

        while (dp->pos < cluster_size)
        {
            DirEntry *dir = (DirEntry *)(cluster_buf + dp->pos);
            dp->pos += sizeof(DirEntry);

            /* End of directory */
            if (dir->DIR_Name[0] == 0x00)
            {
                free(cluster_buf);
                return false;
            }

            /* Deleted entry */
            if (dir->DIR_Name[0] == 0xE5)
                continue;

            /* Skip LFN entries */
            if (dir->DIR_Attr == 0x0F)
                continue;

            /* Skip volume labels */
            if (dir->DIR_Attr & 0x08)
                continue;

            /* Copy name (8.3 format) */
            char name[13];
            memset(name, 0, sizeof(name));

            memcpy(name, dir->DIR_Name, 8);

            for (int i = 7; i >= 0 && name[i] == ' '; i--)
                name[i] = '\0';

            if (dir->DIR_Name[8] != ' ')
            {
                strcat(name, ".");
                char ext[4];
                memcpy(ext, dir->DIR_Name + 8, 3);
                ext[3] = 0;

                for (int i = 2; i >= 0 && ext[i] == ' '; i--)
                    ext[i] = '\0';

                strcat(name, ext);
            }

            strcpy(entry->name, name);

            entry->attr = dir->DIR_Attr;
            entry->size = dir->DIR_FileSize;

            entry->first_cluster =
                ((uint32_t)dir->DIR_FstClusHI << 16) |
                 dir->DIR_FstClusLO;

            free(cluster_buf);
            return true;
        }

        /* Move to next cluster */
        dp->current_cluster = fat32_get_next_cluster(dp->current_cluster);
        dp->pos = 0;
    }

    free(cluster_buf);
    return false;
}


bool f_mkdir(const char *path)
{
    if (!path)
        return false;

    char tmp[256];
    strcpy(tmp, path);

    char *last = strrchr(tmp, '/');

    uint32_t parent_cluster;
    char *dirname;

    if (!last) {
        parent_cluster = fat32_cwd_cluster;
        dirname = tmp;
    }
    else if (last == tmp) {
        parent_cluster = get_root_dir_cluster();
        dirname = last + 1;
    }
    else {
        *last = '\0';
        dirname = last + 1;

        if (!fat32_path_to_cluster(tmp, &parent_cluster))
            return false;
    }

    if (strlen(dirname) == 0)
        return false;

    /* directory already exists */
    if (fat32_dir_exists(parent_cluster, dirname))
        return false;

    return fat32_mkdir_internal(parent_cluster, dirname);
}

bool f_rename(const char *oldpath, const char *newpath)
{
    if (!oldpath || !newpath)
        return false;

    uint32_t old_parent;
    char oldname[256];

    if (!fat32_split_path(oldpath, &old_parent, oldname))
        return false;

    DirEntry entry;
    uint32_t entry_cluster;
    uint32_t entry_offset;

    /* locate existing entry */
    if (!fat32_find_file(old_parent, oldname, &entry, &entry_cluster, &entry_offset))
        return false;

    uint32_t new_parent;
    char newname[256];

    if (!fat32_split_path(newpath, &new_parent, newname))
        return false;

    /* already exists */
    if (fat32_dir_exists(new_parent, newname))
        return false;

    uint32_t first_cluster =
        ((uint32_t)entry.DIR_FstClusHI << 16) |
         entry.DIR_FstClusLO;

    uint8_t attr = entry.DIR_Attr;
    uint32_t size = entry.DIR_FileSize;

    /* create new entry */
    if (!fat32_create_dir_entry(
            new_parent,
            newname,
            attr,
            first_cluster,
            size))
        return false;

    /* mark old entry deleted */
    uint8_t cluster_buf[get_cluster_size_bytes()];

    if (!fat32_read_cluster(entry_cluster, cluster_buf))
        return false;

    DirEntry *dir =
        (DirEntry *)(cluster_buf + entry_offset);

    dir->DIR_Name[0] = 0xE5;

    if (!fat32_write_cluster(entry_cluster, cluster_buf))
        return false;

    return true;
}

bool f_chdir(const char *path)
{
    if (!path)
        return false;

    uint32_t cluster;

    /* resolve path */
    if (!fat32_path_to_cluster(path, &cluster))
        return false;

    /* check that target is a directory */
    if (cluster == 0)
        return false;

    fat32_cwd_cluster = cluster;

    return true;
}



char* f_getcwd(char *buff, uint32_t size)
{
    if (!buff || size == 0)
        return NULL;

    size_t len = strlen(fat32_cwd_path);

    if (len + 1 > size)
        return NULL;

    strcpy(buff, fat32_cwd_path);

    return buff;
}



static bool fat32_match_pattern(const char *pattern, const char *name)
{
    while (*pattern && *name) {

        if (*pattern == '*') {
            pattern++;

            if (!*pattern)
                return true;

            while (*name) {
                if (fat32_match_pattern(pattern, name))
                    return true;
                name++;
            }

            return false;
        }

        if (*pattern != '?' && tolower(*pattern) != tolower(*name))
            return false;

        pattern++;
        name++;
    }

    if (*pattern == '*')
        pattern++;

    return (*pattern == 0 && *name == 0);
}

bool f_findfirst(FAT32_DIR *dp, FAT32_DIRENT *entry, const char *path, const char *pattern)
{
    if (!dp || !entry || !path || !pattern)
        return false;

    if (!f_opendir(dp, (char*)path))
        return false;

    while (f_readdir(dp, entry)) {

        if (fat32_match_pattern(pattern, entry->name))
            return true;
    }

    return false;
}


bool f_findnext(FAT32_DIR *dp, FAT32_DIRENT *entry,  const char *pattern)
{
    if (!dp || !entry || !pattern)
        return false;

    while (f_readdir(dp, entry)) {

        if (fat32_match_pattern(pattern, entry->name))
            return true;
    }

    return false;
}




