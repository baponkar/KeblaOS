#pragma once

#include <stdint.h>


struct disk{
    uint32_t controller; // In a real driver, this might be a pointer to an AHCI controller structure.
    uint32_t port;       // Port number on the AHCI controller.
};
typedef struct disk disk_t;

void get_disk_info();
disk_t* get_disk(int disk_index);




