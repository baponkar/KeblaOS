#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef uint8_t guid_t[16]; // Globally Unique Identifier

// Predefined GUIDs for partition types
extern const guid_t ESP_TYPE_GUID;
extern const guid_t LINUX_FS_GUID;


// Windows specific GUIDs
extern const guid_t MS_RESERVED_GUID;
extern const guid_t WINDOWS_MSR_GUID;
extern const guid_t WINDOWS_RECOVERY_GUID;

// Example GUIDs for testing
extern const guid_t DISK_GUID_EXAMPLE;
extern const guid_t ESP_GUID;
extern const guid_t DATA_PARTITION_GUID;



void guid_to_string(uint8_t guid[16], char *out);
void generate_guid(uint8_t guid[16]);








