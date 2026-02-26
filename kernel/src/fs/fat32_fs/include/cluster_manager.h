#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


#include "fat32_types.h"


bool fat32_zero_cluster(uint32_t cluster_no);
bool fat32_read_cluster(uint32_t cluster_number, void *buffer);
bool fat32_write_cluster( uint32_t cluster_number, const void *buffer);
bool fat32_clear_cluster( uint32_t cluster);
uint32_t fat32_get_next_cluster( uint32_t current_cluster);
bool fat32_set_next_cluster( uint32_t current_cluster, uint32_t next_cluster);
bool fat32_validate_cluster_chain( uint32_t start_cluster);
bool fat32_free_cluster_chain( uint32_t start_cluster);
bool fat32_allocate_cluster( uint32_t *allocated_cluster);
bool fat32_allocate_cluster_chain( uint32_t count, uint32_t *first_cluster);
bool fat32_read_cluster_chain( uint32_t start_cluster, void *buffer, uint32_t max_bytes);
bool fat32_write_cluster_chain( const void *buffer, uint32_t size, uint32_t *first_cluster);
uint32_t fat32_count_cluster_chain( uint32_t start_cluster);
bool fat32_append_cluster( uint32_t start_cluster, uint32_t *new_cluster);
bool fat32_find_free_dir_entry( uint32_t dir_cluster, uint32_t *out_cluster, uint32_t *out_offset);
void fat32_format_83_name(const char *name, char out[11]);
bool fat32_set_volume_label( const char *label);
bool fat32_create_dir_entry( uint32_t parent_cluster, const char *name, uint8_t attr, uint32_t first_cluster , uint32_t file_size);
bool fat32_init_directory( uint32_t dir_cluster, uint32_t parent_cluster);
bool fat32_mkdir_internal( uint32_t parent_cluster, const char *name);
bool fat32_dir_exists( uint32_t dir_cluster, const char *name);
bool fat32_find_dir( uint32_t dir_cluster, const char *name, uint32_t *out_cluster);
bool fat32_create_file_in_dir( uint32_t parent_cluster, const char *filename, const char *content, uint32_t size);
bool fat32_path_to_cluster( const char *path, uint32_t *out_cluster);
bool fat32_find_file(  uint32_t dir_cluster, const char *name, DirEntry *out_entry, uint32_t *entry_cluster, uint32_t *entry_offset);










