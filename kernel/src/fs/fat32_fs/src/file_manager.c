
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "../include/cluster_manager.h"
#include "../include/fat32_bpb.h"

#include "../include/file_manager.h"

extern uint32_t fat32_cwd_cluster;

bool f_open( FAT32_FILE* fp, const char* path, int mode){
    if (!fp || !path)
        return false;

    char tmp[256];
    strcpy(tmp, path);

    char *last = strrchr(tmp, '/');

    uint32_t parent_cluster;
    char *filename;

    /* resolve parent directory */
    if (!last) {
        parent_cluster = fat32_cwd_cluster;
        filename = tmp;
    }
    else if (last == tmp) {
        parent_cluster = get_root_dir_cluster();
        filename = last + 1;
    }
    else {
        *last = '\0';
        filename = last + 1;

        if (!fat32_path_to_cluster(tmp, &parent_cluster))
            return false;
    }

    DirEntry entry;
    uint32_t entry_cluster;
    uint32_t entry_offset;

    bool exists =
        fat32_find_file(
            parent_cluster,
            filename,
            &entry,
            &entry_cluster,
            &entry_offset
        );

    /* create file if not exists */
    if (!exists)
    {
        if (!(mode & (FA_CREATE | FA_CREATE_ALWAYS)))
            return false;

        if (!fat32_create_dir_entry(
                parent_cluster,
                filename,
                0x20,      /* archive attribute */
                0,
                0))
            return false;

        /* find the newly created entry */
        if (!fat32_find_file(
                parent_cluster,
                filename,
                &entry,
                &entry_cluster,
                &entry_offset))
            return false;
    }

    /* fill file structure */
    fp->first_cluster =
        ((uint32_t)entry.DIR_FstClusHI << 16) |
         entry.DIR_FstClusLO;

    fp->current_cluster = fp->first_cluster;

    fp->size = entry.DIR_FileSize;
    fp->pos = 0;

    fp->parent_cluster = parent_cluster;

    fp->dir_entry_sector = entry_cluster;
    fp->dir_entry_offset = entry_offset;

    strncpy(fp->name, filename, sizeof(fp->name)-1);
    fp->name[sizeof(fp->name)-1] = '\0';

    fp->mode = mode;

    return true;
}



bool f_close(FAT32_FILE* fp)
{
    if (!fp)
        return false;

    uint32_t cluster_size = get_cluster_size_bytes();

    uint8_t *cluster_buf = malloc(cluster_size);
    if (!cluster_buf)
        return false;

    /* read directory cluster containing the entry */
    if (!fat32_read_cluster(fp->dir_entry_sector, cluster_buf)) {
        free(cluster_buf);
        return false;
    }

    DirEntry *entry =
        (DirEntry *)(cluster_buf + fp->dir_entry_offset);

    /* update file size */
    entry->DIR_FileSize = fp->size;

    /* update cluster */
    entry->DIR_FstClusHI = (fp->first_cluster >> 16) & 0xFFFF;
    entry->DIR_FstClusLO = (fp->first_cluster & 0xFFFF);

    /* write updated entry back */
    if (!fat32_write_cluster(fp->dir_entry_sector, cluster_buf)) {
        free(cluster_buf);
        return false;
    }

    free(cluster_buf);

    /* reset file object */
    fp->first_cluster = 0;
    fp->current_cluster = 0;
    fp->size = 0;
    fp->pos = 0;
    fp->parent_cluster = 0;
    fp->dir_entry_sector = 0;
    fp->dir_entry_offset = 0;
    fp->name[0] = '\0';
    fp->mode = 0;

    return true;
}

bool f_read(FAT32_FILE* fp, void *buff, uint32_t btr, uint32_t *br)
{
    if (!fp || !buff || !br)
        return false;

    *br = 0;

    if (fp->pos >= fp->size)
        return true;

    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *cluster_buf = malloc(cluster_size);

    if (!cluster_buf)
        return false;

    uint32_t remaining = fp->size - fp->pos;
    if (btr > remaining)
        btr = remaining;

    uint32_t current_cluster = fp->first_cluster;
    uint32_t offset = fp->pos;

    /* move to correct cluster */
    while (offset >= cluster_size && current_cluster)
    {
        offset -= cluster_size;
        current_cluster = fat32_get_next_cluster(current_cluster);
    }

    while (*br < btr && current_cluster)
    {
        if (!fat32_read_cluster(current_cluster, cluster_buf))
        {
            free(cluster_buf);
            return false;
        }

        uint32_t copy_offset = offset;
        uint32_t copy_size = cluster_size - copy_offset;

        if (copy_size > (btr - *br))
            copy_size = btr - *br;

        memcpy(
            (uint8_t*)buff + *br,
            cluster_buf + copy_offset,
            copy_size
        );

        *br += copy_size;
        offset = 0;

        current_cluster = fat32_get_next_cluster(current_cluster);
    }

    fp->pos += *br;

    free(cluster_buf);

    return true;
}



bool f_write(FAT32_FILE* fp, const void* buff, uint32_t btw, uint32_t* bw)
{
    if (!fp || !buff || !bw)
        return false;

    *bw = 0;

    uint32_t cluster_size = get_cluster_size_bytes();

    uint8_t *cluster_buf = malloc(cluster_size);
    if (!cluster_buf) return false;

    uint32_t current_cluster = fp->first_cluster;
    uint32_t offset = fp->pos;

    /* allocate first cluster if file empty */
    if (current_cluster == 0)
    {
        if (!fat32_allocate_cluster(&current_cluster)) {
            free(cluster_buf);
            return false;
        }

        fp->first_cluster = current_cluster;
        fp->current_cluster = current_cluster;
    }

    /* walk to correct cluster */
    while (offset >= cluster_size)
    {
        uint32_t next = fat32_get_next_cluster(current_cluster);

        if (next >= 0x0FFFFFF8)
        {
            if (!fat32_append_cluster(current_cluster, &next)) {
                free(cluster_buf);
                return false;
            }
        }

        current_cluster = next;
        offset -= cluster_size;
    }


    while (*bw < btw)
    {
        if (!fat32_read_cluster(current_cluster, cluster_buf)) {
            free(cluster_buf);
            return false;
        }

        uint32_t write_offset = offset;
        uint32_t write_bytes = cluster_size - write_offset;

        if (write_bytes > (btw - *bw)){
            write_bytes = btw - *bw;
        }
            
        memcpy( cluster_buf + write_offset, (uint8_t*)buff + *bw, write_bytes );

        if (!fat32_write_cluster(current_cluster, cluster_buf)) {
            free(cluster_buf);
            return false;
        }

        *bw += write_bytes;
        offset = 0;

        if (*bw >= btw) break;

        uint32_t next = fat32_get_next_cluster(current_cluster);

        if (next >= 0x0FFFFFF8)
        {
            if (!fat32_append_cluster(current_cluster, &next)) {
                free(cluster_buf);
                return false;
            }
        }

        current_cluster = next;
    }


    fp->pos += *bw;

    if (fp->pos > fp->size) fp->size = fp->pos;

    fp->current_cluster = current_cluster;

    free(cluster_buf);

    return true;
}




bool f_lseek(FAT32_FILE* fp, uint32_t ofs)
{
    if (!fp)
        return false;

    uint32_t cluster_size = get_cluster_size_bytes();

    /* clamp offset if beyond file size */
    if (ofs > fp->size)
        ofs = fp->size;

    uint32_t cluster = fp->first_cluster;
    uint32_t remaining = ofs;

    /* walk cluster chain */
    while (remaining >= cluster_size && cluster)
    {
        cluster = fat32_get_next_cluster(cluster);
        remaining -= cluster_size;
    }

    fp->pos = ofs;
    fp->current_cluster = cluster;

    return true;
}

bool f_truncate(FAT32_FILE* fp)
{
    if (!fp)
        return false;

    uint32_t cluster_size = get_cluster_size_bytes();

    /* if file becomes empty */
    if (fp->pos == 0)
    {
        if (fp->first_cluster)
            fat32_free_cluster_chain(fp->first_cluster);

        fp->first_cluster = 0;
        fp->current_cluster = 0;
        fp->size = 0;

        return true;
    }

    uint32_t cluster = fp->first_cluster;
    uint32_t remaining = fp->pos;

    /* walk to the cluster containing the truncation point */
    while (remaining >= cluster_size && cluster)
    {
        remaining -= cluster_size;
        cluster = fat32_get_next_cluster(cluster);
    }

    if (!cluster)
        return false;

    uint32_t next = fat32_get_next_cluster(cluster);

    /* terminate cluster chain */
    fat32_set_next_cluster(cluster, 0x0FFFFFFF);   // FAT32 EOC

    /* free remaining clusters */
    if (next)
        fat32_free_cluster_chain(next);

    fp->size = fp->pos;
    fp->current_cluster = cluster;

    return true;
}

bool f_sync(FAT32_FILE *fp)
{
    if (!fp)
        return false;

    uint32_t cluster_size = get_cluster_size_bytes();

    uint8_t *cluster_buf = malloc(cluster_size);
    if (!cluster_buf)
        return false;

    /* read cluster containing directory entry */
    if (!fat32_read_cluster(fp->dir_entry_sector, cluster_buf)) {
        free(cluster_buf);
        return false;
    }

    DirEntry *entry =
        (DirEntry *)(cluster_buf + fp->dir_entry_offset);

    /* update file size */
    entry->DIR_FileSize = fp->size;

    /* update first cluster */
    entry->DIR_FstClusHI = (fp->first_cluster >> 16) & 0xFFFF;
    entry->DIR_FstClusLO = fp->first_cluster & 0xFFFF;

    /* write updated directory entry back */
    if (!fat32_write_cluster(fp->dir_entry_sector, cluster_buf)) {
        free(cluster_buf);
        return false;
    }

    free(cluster_buf);

    return true;
}


bool f_forward(
    FAT32_FILE *fp,
    uint32_t (*func)(const uint8_t *data, uint32_t len),
    uint32_t btf,
    uint32_t *bf
){
    if (!fp || !func || !bf)
        return false;

    *bf = 0;

    if (fp->pos >= fp->size)
        return true;

    uint32_t cluster_size = get_cluster_size_bytes();

    uint8_t *cluster_buf = malloc(cluster_size);
    if (!cluster_buf)
        return false;

    uint32_t remaining = fp->size - fp->pos;

    if (btf > remaining)
        btf = remaining;

    uint32_t cluster = fp->first_cluster;
    uint32_t offset = fp->pos;

    /* move to correct cluster */
    while (offset >= cluster_size && cluster)
    {
        offset -= cluster_size;
        cluster = fat32_get_next_cluster(cluster);
    }

    while (*bf < btf && cluster)
    {
        if (!fat32_read_cluster(cluster, cluster_buf))
        {
            free(cluster_buf);
            return false;
        }

        uint32_t copy_offset = offset;
        uint32_t forward_size = cluster_size - copy_offset;

        if (forward_size > (btf - *bf))
            forward_size = btf - *bf;

        uint32_t sent =
            func(cluster_buf + copy_offset, forward_size);

        *bf += sent;

        if (sent < forward_size)
            break;

        offset = 0;
        cluster = fat32_get_next_cluster(cluster);
    }

    fp->pos += *bf;

    free(cluster_buf);

    return true;
}

bool f_expand(FAT32_FILE *fp, uint32_t size)
{
    if (!fp)
        return false;

    uint32_t cluster_size = get_cluster_size_bytes();

    /* clusters needed */
    uint32_t needed =
        (size + cluster_size - 1) / cluster_size;

    uint32_t current = 0;

    if (fp->first_cluster)
        current = fat32_count_cluster_chain(fp->first_cluster);

    /* already large enough */
    if (current >= needed)
        return true;

    uint32_t to_allocate = needed - current;

    uint32_t first_new = 0;

    if (!fat32_allocate_cluster_chain(to_allocate, &first_new))
        return false;

    /* file had no cluster yet */
    if (fp->first_cluster == 0)
    {
        fp->first_cluster = first_new;
        fp->current_cluster = first_new;
        return true;
    }

    /* append chain to file */
    uint32_t last = fp->first_cluster;

    while (fat32_get_next_cluster(last))
        last = fat32_get_next_cluster(last);

    fat32_set_next_cluster(last, first_new);

    return true;
}

char* f_gets(char *buff, int len, FAT32_FILE *fp)
{
    if (!buff || !fp || len <= 0)
        return NULL;

    int i = 0;
    uint32_t br;
    char c;

    while (i < (len - 1))
    {
        if (!f_read(fp, &c, 1, &br) || br == 0)
        {
            if (i == 0)
                return NULL;   // EOF
            break;
        }

        buff[i++] = c;

        if (c == '\n')
            break;
    }

    buff[i] = '\0';

    return buff;
}

int f_putc(char c, FAT32_FILE *fp)
{
    if (!fp)
        return -1;  // EOF

    uint32_t bw;

    if (!f_write(fp, &c, 1, &bw))
        return -1;

    if (bw != 1)
        return -1;

    return (unsigned char)c;
}


int f_puts(const char *str, FAT32_FILE *fp)
{
    if (!str || !fp)
        return -1;   // EOF

    uint32_t len = strlen(str);
    uint32_t bw = 0;

    if (!f_write(fp, str, len, &bw))
        return -1;

    if (bw != len)
        return -1;

    return (int)bw;
}




int f_printf(FAT32_FILE *fp, const char *fmt, ...)
{
    if (!fp || !fmt)
        return -1;

    char buffer[1024];

    va_list args;
    va_start(args, fmt);

    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);

    va_end(args);

    if (len < 0)
        return -1;

    uint32_t bw = 0;

    if (!f_write(fp, buffer, len, &bw))
        return -1;

    if (bw != (uint32_t)len)
        return -1;

    return len;
}

uint32_t f_tell(FAT32_FILE *fp)
{
    if (!fp)
        return 0;

    return fp->pos;
}

uint32_t f_size(FAT32_FILE *fp)
{
    if (!fp)
        return 0;

    return fp->size;
}

bool f_eof(FAT32_FILE *fp)
{
    if (!fp)
        return true;

    return (fp->pos >= fp->size);
}

bool f_stat(const char *path, FAT32_STAT *stat)
{
    if (!path || !stat)
        return false;

    char tmp[256];
    strcpy(tmp, path);

    char *last = strrchr(tmp, '/');

    uint32_t parent_cluster;
    char *filename;

    /* resolve parent directory */
    if (!last) {
        parent_cluster = fat32_cwd_cluster;
        filename = tmp;
    }
    else if (last == tmp) {
        parent_cluster = get_root_dir_cluster();
        filename = last + 1;
    }
    else {
        *last = '\0';
        filename = last + 1;

        if (!fat32_path_to_cluster(tmp, &parent_cluster))
            return false;
    }

    DirEntry entry;
    uint32_t entry_cluster;
    uint32_t entry_offset;

    if (!fat32_find_file(
            parent_cluster,
            filename,
            &entry,
            &entry_cluster,
            &entry_offset))
        return false;

    /* fill stat structure */
    strcpy(stat->name, filename);

    stat->attr = entry.DIR_Attr;

    stat->size = entry.DIR_FileSize;

    stat->first_cluster =
        ((uint32_t)entry.DIR_FstClusHI << 16) |
         entry.DIR_FstClusLO;

    return true;
}

bool f_unlink(const char *path)
{
    if (!path)
        return false;

    char tmp[256];
    strcpy(tmp, path);

    char *last = strrchr(tmp, '/');

    uint32_t parent_cluster;
    char *filename;

    /* resolve parent directory */
    if (!last) {
        parent_cluster = fat32_cwd_cluster;
        filename = tmp;
    }
    else if (last == tmp) {
        parent_cluster = get_root_dir_cluster();
        filename = last + 1;
    }
    else {
        *last = '\0';
        filename = last + 1;

        if (!fat32_path_to_cluster(tmp, &parent_cluster))
            return false;
    }

    DirEntry entry;
    uint32_t entry_cluster;
    uint32_t entry_offset;

    if (!fat32_find_file(
            parent_cluster,
            filename,
            &entry,
            &entry_cluster,
            &entry_offset))
        return false;

    /* get first cluster */
    uint32_t first_cluster =
        ((uint32_t)entry.DIR_FstClusHI << 16) |
         entry.DIR_FstClusLO;

    /* free cluster chain */
    if (first_cluster)
        fat32_free_cluster_chain(first_cluster);

    /* read directory cluster */
    uint32_t cluster_size = get_cluster_size_bytes();
    uint8_t *cluster_buf = malloc(cluster_size);

    if (!cluster_buf)
        return false;

    if (!fat32_read_cluster(entry_cluster, cluster_buf)) {
        free(cluster_buf);
        return false;
    }

    /* mark entry deleted */
    DirEntry *dir =
        (DirEntry *)(cluster_buf + entry_offset);

    dir->DIR_Name[0] = 0xE5;

    if (!fat32_write_cluster(entry_cluster, cluster_buf)) {
        free(cluster_buf);
        return false;
    }

    free(cluster_buf);

    return true;
}


int f_error(FAT32_FILE *fp)
{
    if (!fp)
        return -1;

    return fp->error;
}