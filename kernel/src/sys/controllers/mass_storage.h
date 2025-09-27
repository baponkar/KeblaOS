#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    MASS_STORAGE_SUBCLASS_IDE = 0x1,
    MASS_STORAGE_SUBCLASS_FLOPY_DISK = 0x2,
    MASS_STORAGE_SUBCLASS_IPI_BUS = 0x3,
    MASS_STORAGE_SUBCLASS_RAID = 0x4,
    MASS_STORAGE_SUBCLASS_ATA = 0x5,
    MASS_STORAGE_SUBCLASS_SERIAL_ATA = 0x6,
    MASS_STORAGE_SUBCLASS_SCSI = 0x7,
    MASS_STORAGE_SUBCLASS_NON_VOLATILE_MEM = 0x8,
    MASS_STORAGE_SUBCLASS_OTHER = 0x80
}MASS_STORAGE_SUBCLASS_CODE;

extern pci_device_t* mass_storage_controllers;     // Array to store detected mass storage devices
extern int mass_storage_controllers_count;         // Counter for mass storage devices

void alloc_mass_storage_controllers_memory();

int get_mass_storage_controllers_count();
pci_device_t* get_mass_storage_controller(int index);
