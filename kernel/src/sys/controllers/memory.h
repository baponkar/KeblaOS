#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>



typedef enum {
    MEMORY_SUBCLASS_RAM = 0x0,
    MEMORY_SUBCLASS_FLASH = 0x1,
    MEMORY_SUBCLASS_OTHER = 0x80
}MEMORY_SUBCLASS_CODE;

extern pci_device_t *memory_controllers;         // Array to store detected memory controllers
extern int memory_controllers_count;             // Counter for memory controllers

void alloc_memory_controllers_memory();
int get_memory_controllers_count();
pci_device_t* get_memory_controller(int index);

