#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    SERIAL_BUS_SUBCLASS_FIREWIRE = 0x0,
    SERIAL_BUS_SUBCLASS_ACCESS = 0x1,
    SERIAL_BUS_SUBCLASS_SSA = 0x2,
    SERIAL_BUS_SUBCLASS_USB = 0x3,
    SERIAL_BUS_SUBCLASS_FIBRE = 0x4,
    SERIAL_BUS_SUBCLASS_SMBUS = 0x5,
    SERIAL_BUS_SUBCLASS_INFINI_BAND = 0x6,
    SERIAL_BUS_SUBCLASS_IPMI = 0x7,
    SERIAL_BUS_SUBCLASS_SERCOS = 0x8,
    SERIAL_BUS_SUBCLASS_CANBUS = 0x9,
    SERIAL_BUS_SUBCLASS_OTHER = 0x80
}SERIAL_BUS_SUBCLASS_CODE;

extern pci_device_t *serial_bus_controllers;         // Array to store detected serial bus controllers
extern int serial_bus_controllers_count;             // Counter for serial bus controllers

void alloc_serial_bus_controllers_memory();
int get_serial_bus_controllers_count();
pci_device_t* get_serial_bus_controller(int index);

void detected_serial_bus_controller_info();



