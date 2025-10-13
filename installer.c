#include "../fs/fatfs_wrapper.h"
#include "../fs/iso9660.h"

#include "../fs/FatFs-R0.15b/source/ff.h"

#include "installer.h"



// Initialize installer
installer_context_t* installer_init(HBA_PORT_T *cdrom_port, int target_disk, 
                                   progress_callback_t progress_cb, 
                                   status_callback_t status_cb) {
    
    installer_context_t *ctx = malloc(sizeof(installer_context_t));
    if (!ctx) return NULL;
    
    // Initialize ISO context
    ctx->iso_ctx = malloc(sizeof(iso9660_context_t));
    if (!ctx->iso_ctx) {
        free(ctx);
        return NULL;
    }
    
    if (!iso9660_init(cdrom_port, ctx->iso_ctx)) {
        free(ctx->iso_ctx);
        free(ctx);
        return NULL;
    }
    
    // Initialize FATFS on target disk
    if (fatfs_init(target_disk) != 0) {
        free(ctx->iso_ctx);
        free(ctx);
        return NULL;
    }
    
    if (fatfs_mount(target_disk) != 0) {
        free(ctx->iso_ctx);
        free(ctx);
        return NULL;
    }
    
    ctx->target_disk_no = target_disk;
    ctx->progress_cb = progress_cb;
    ctx->status_cb = status_cb;
    ctx->total_files = 0;
    ctx->copied_files = 0;
    ctx->cancel_requested = false;
    
    return ctx;
}

// Cleanup installer
void installer_cleanup(installer_context_t *ctx) {
    if (ctx) {
        if (ctx->iso_ctx) {
            free(ctx->iso_ctx);
        }
        free(ctx);
    }
}

// Create directory structure on target
static bool create_target_directory(const char *path) {
    char temp[256];
    char *p;
    
    // Skip root
    if (path[0] == '/') {
        strncpy(temp, path + 1, sizeof(temp) - 1);
    } else {
        strncpy(temp, path, sizeof(temp) - 1);
    }
    
    temp[sizeof(temp) - 1] = '\0';
    
    // Create each directory in the path
    p = temp;
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            if (temp[0] != '\0') {
                if (fatfs_mkdir(temp) != 0) {
                    // Directory might already exist, continue
                }
            }
            *p = '/';
        }
        p++;
    }
    
    // Create the final directory
    if (temp[0] != '\0') {
        if (fatfs_mkdir(temp) != 0) {
            // Directory might already exist
        }
    }
    
    return true;
}

// Copy single file with progress reporting
static bool copy_single_file(installer_context_t *ctx, const char *src_path, const char *dest_path) {
    void *file_buffer = NULL;
    uint32_t file_size = 0;
    
    if (ctx->status_cb) {
        ctx->status_cb(src_path);
    }
    
    // Read file from ISO
    if (!iso9660_copy_file(ctx->iso_ctx, src_path, &file_buffer, &file_size)) {
        if (ctx->status_cb) {
            ctx->status_cb("Failed to read file from ISO");
        }
        return false;
    }
    
    // Create target directory if needed
    char dir_path[256];
    const char *last_slash = strrchr(dest_path, '/');
    if (last_slash) {
        int dir_len = last_slash - dest_path;
        if (dir_len > 0 && dir_len < (int)sizeof(dir_path)) {
            strncpy(dir_path, dest_path, dir_len);
            dir_path[dir_len] = '\0';
            create_target_directory(dir_path);
        }
    }
    
    // Write file to FATFS
    void *fp = fatfs_open(dest_path, FA_CREATE_ALWAYS | FA_WRITE);
    if (!fp) {
        if (ctx->status_cb) {
            ctx->status_cb("Failed to create target file");
        }
        free(file_buffer);
        return false;
    }
    
    // Write in chunks to support progress reporting
    const uint32_t CHUNK_SIZE = 4096;
    uint32_t bytes_written = 0;
    uint32_t chunk_size;
    
    while (bytes_written < file_size && !ctx->cancel_requested) {
        chunk_size = (file_size - bytes_written > CHUNK_SIZE) ? CHUNK_SIZE : (file_size - bytes_written);
        
        // Note: This assumes fatfs_write can handle binary data
        // You might need to implement a binary write function
        if (fatfs_write(fp, (char*)((uint8_t*)file_buffer + bytes_written)) != 0) {
            fatfs_close(fp);
            free(file_buffer);
            return false;
        }
        
        bytes_written += chunk_size;
        
        // Report progress for this file
        if (ctx->progress_cb) {
            uint32_t percentage = (bytes_written * 100) / file_size;
            ctx->progress_cb(src_path, bytes_written, file_size, percentage);
        }
    }
    
    fatfs_close(fp);
    fatfs_sync(fp); // Ensure data is written to disk
    free(file_buffer);
    
    if (ctx->cancel_requested) {
        return false;
    }
    
    ctx->copied_files++;
    
    // Report overall progress
    if (ctx->progress_cb && ctx->total_files > 0) {
        uint32_t overall_percentage = (ctx->copied_files * 100) / ctx->total_files;
        ctx->progress_cb("Overall progress", ctx->copied_files, ctx->total_files, overall_percentage);
    }
    
    return true;
}

// Get list of files from ISO (simplified - you'd need to implement proper directory traversal)
static uint32_t get_file_list(iso9660_context_t *ctx, file_entry_t **file_list) {
    // This is a simplified implementation
    // In a real scenario, you'd traverse the ISO directory structure
    // and build a complete file list
    
    static const char* essential_files[] = {
        "/kernel.bin",
        "/initrd.img",
        "/system/bootloader.bin",
        "/system/config.sys",
        "/drivers/disk.sys",
        "/drivers/display.sys",
        "/bin/sh",
        "/bin/ls",
        "/bin/cat",
        NULL
    };
    
    uint32_t count = 0;
    while (essential_files[count] != NULL) {
        count++;
    }
    
    *file_list = malloc(count * sizeof(file_entry_t));
    if (!*file_list) return 0;
    
    for (uint32_t i = 0; i < count; i++) {
        (*file_list)[i].source_path = strdup(essential_files[i]);
        (*file_list)[i].target_path = strdup(essential_files[i]);
        (*file_list)[i].size = 0; // Would be populated from actual file info
    }
    
    return count;
}

// Free file list
static void free_file_list(file_entry_t *file_list, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        free(file_list[i].source_path);
        free(file_list[i].target_path);
    }
    free(file_list);
}

// Format target disk
static bool format_target_disk(installer_context_t *ctx) {
    if (ctx->status_cb) {
        ctx->status_cb("Formatting target disk...");
    }
    
    // Unmount first
    fatfs_chdrive("0:");
    
    // Format the disk (adjust parameters as needed)
    if (fatfs_mkfs(ctx->target_disk_no, 0) != 0) {
        if (ctx->status_cb) {
            ctx->status_cb("Format failed!");
        }
        return false;
    }
    
    // Remount
    if (fatfs_mount(ctx->target_disk_no) != 0) {
        if (ctx->status_cb) {
            ctx->status_cb("Remount failed after format!");
        }
        return false;
    }
    
    if (ctx->status_cb) {
        ctx->status_cb("Format completed successfully");
    }
    
    return true;
}

// Platform-specific boot sector installation
static void install_boot_sector(installer_context_t *ctx) {
    // This is highly platform-specific
    // You would need to:
    // 1. Read the boot sector from ISO
    // 2. Modify it for the target disk
    // 3. Write it to the first sector of the target disk
    
    // Example placeholder:
    if (ctx->status_cb) {
        ctx->status_cb("Boot sector installation would go here");
    }
}

// Main installation function
bool install_operating_system(installer_context_t *ctx, bool format_disk) {
    if (!ctx) return false;
    
    if (ctx->status_cb) {
        ctx->status_cb("Starting OS installation...");
    }
    
    // Step 1: Format target disk if requested
    if (format_disk) {
        if (!format_target_disk(ctx)) {
            return false;
        }
    }
    
    // Step 2: Get file list from ISO
    file_entry_t *file_list = NULL;
    ctx->total_files = get_file_list(ctx->iso_ctx, &file_list);
    
    if (ctx->total_files == 0) {
        if (ctx->status_cb) {
            ctx->status_cb("No files found to install!");
        }
        return false;
    }
    
    if (ctx->status_cb) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Found %u files to install", ctx->total_files);
        ctx->status_cb(msg);
    }
    
    // Step 3: Copy all files
    bool success = true;
    for (uint32_t i = 0; i < ctx->total_files && !ctx->cancel_requested; i++) {
        if (!copy_single_file(ctx, file_list[i].source_path, file_list[i].target_path)) {
            success = false;
            break;
        }
    }
    
    // Step 4: Create boot sector or bootloader if needed
    if (success && !ctx->cancel_requested) {
        if (ctx->status_cb) {
            ctx->status_cb("Installing bootloader...");
        }
        
        // Here you would install the boot sector
        // This is platform-specific and would require low-level disk writing
        install_boot_sector(ctx);
    }
    
    // Cleanup
    free_file_list(file_list);
    
    if (ctx->cancel_requested) {
        if (ctx->status_cb) {
            ctx->status_cb("Installation cancelled by user");
        }
        return false;
    }
    
    if (success) {
        if (ctx->status_cb) {
            ctx->status_cb("Installation completed successfully!");
        }
    } else {
        if (ctx->status_cb) {
            ctx->status_cb("Installation failed!");
        }
    }
    
    return success;
}

// Cancel installation
void cancel_installation(installer_context_t *ctx) {
    if (ctx) {
        ctx->cancel_requested = true;
    }
}

// Get installation progress
void get_installation_progress(installer_context_t *ctx, uint32_t *current, uint32_t *total, uint32_t *percentage) {
    if (ctx) {
        *current = ctx->copied_files;
        *total = ctx->total_files;
        *percentage = (ctx->total_files > 0) ? (ctx->copied_files * 100) / ctx->total_files : 0;
    } else {
        *current = *total = *percentage = 0;
    }
}



// Example usage and callbacks
void example_progress_callback(const char* filename, uint32_t current, uint32_t total, uint32_t percentage) {
    // Display progress to user
    printf("Copying %s: %u/%u bytes (%u%%)\n", filename, current, total, percentage);
}

void example_status_callback(const char* message) {
    // Display status message
    printf("[STATUS] %s\n", message);
}

// Example main function
void example_installer_usage(HBA_PORT_T *cdrom_port, int target_disk) {
    // Create installer context
    installer_context_t *installer = installer_init(cdrom_port, target_disk, 
                                                   example_progress_callback, 
                                                   example_status_callback);
    
    if (!installer) {
        printf("Failed to initialize installer!\n");
        return;
    }
    
    // Start installation (format disk first)
    bool success = install_operating_system(installer, true);
    
    if (success) {
        printf("OS installed successfully!\n");
    } else {
        printf("OS installation failed!\n");
    }
    
    // Cleanup
    installer_cleanup(installer);
}









