#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>



typedef enum {
    INPUT_DEVICE_SUBCLASS_KEYBOARD = 0x0,
    INPUT_DEVICE_SUBCLASS_DIGITIZER_PEN = 0x1,
    INPUT_DEVICE_SUBCLASS_MOUSE = 0x2,
    INPUT_DEVICE_SUBCLASS_SCANNER = 0x3,
    INPUT_DEVICE_SUBCLASS_GAMEPORT = 0x4,
    INPUT_DEVICE_SUBCLASS_OTHER = 0x80
}INPUT_DEVICE_SUBCLASS_CODE;

extern pci_device_t *input_devices;         // Array to store detected input devices
extern int input_devices_count;             // Counter for input devices

void alloc_input_devices_memory();
int get_input_devices_count();
pci_device_t* get_input_device(int index);

void detected_input_device_info();

