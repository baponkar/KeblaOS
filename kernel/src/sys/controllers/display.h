#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../../driver/pci/pci.h"

typedef enum {
    DISPLAY_SUBCLASS_VGA = 0x0,
    DISPLAY_SUBCLASS_XGA = 0x1,
    DISPLAY_SUBCLASS_3D = 0x2,
    DISPLAY_SUBCLASS_OTHER = 0x80
}DISPLAY_SUBCLASS_CODE;

extern pci_device_t *display_controllers;         // Array to store detected display controllers
extern int display_controllers_count;             // Counter for display controllers

void alloc_display_controllers_memory();
int get_display_controllers_count();
pci_device_t* get_display_controller(int index);

