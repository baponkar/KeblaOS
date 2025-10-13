#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stddef.h>



#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"


// Progress callback types
typedef void (*progress_callback_t)(const char* filename, uint32_t current, uint32_t total, uint32_t percentage);
typedef void (*status_callback_t)(const char* message);

// Installer context structure
typedef struct {
    iso9660_context_t *iso_ctx;
    int target_disk_no;
    progress_callback_t progress_cb;
    status_callback_t status_cb;
    uint32_t total_files;
    uint32_t copied_files;
    bool cancel_requested;
} installer_context_t;

// File copy structure
typedef struct {
    char *source_path;
    char *target_path;
    uint32_t size;
} file_entry_t;



installer_context_t* installer_init(HBA_PORT_T *cdrom_port, int target_disk, progress_callback_t progress_cb, status_callback_t status_cb);
void installer_cleanup(installer_context_t *ctx);
bool install_operating_system(installer_context_t *ctx, bool format_disk);
void cancel_installation(installer_context_t *ctx);
void get_installation_progress(installer_context_t *ctx, uint32_t *current, uint32_t *total, uint32_t *percentage);

void example_progress_callback(const char* filename, uint32_t current, uint32_t total, uint32_t percentage);
void example_status_callback(const char* message);
void example_installer_usage(HBA_PORT_T *cdrom_port, int target_disk);





