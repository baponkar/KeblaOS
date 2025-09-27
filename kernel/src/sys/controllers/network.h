#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


typedef enum {
    NETWORK_SUBCLASS_ETHERNET = 0x0,
    NETWORK_SUBCLASS_TOKEN_RING = 0x1,
    NETWORK_SUBCLASS_FDDI = 0x2,
    NETWORK_SUBCLASS_ATM = 0x3,
    NETWORK_SUBCLASS_ISDN = 0x4,
    NETWORK_SUBCLASS_WORLD_FIP = 0x5,
    NETWORK_SUBCLASS_PIC_MG = 0x6,
    NETWORK_SUBCLASS_INFINBAND = 0x7,
    NETWORK_SUBCLASS_FABRIC = 0x8,
    NETWORK_SUBCLASS_OTHER = 0x80
}NETWORK_SUBCLASS_CODE;

extern pci_device_t *network_controllers;         // Array to store detected network controllers
extern int network_controllers_count;             // Counter for network controllers

void alloc_network_controllers_memory();
int get_network_controllers_count();


pci_device_t* get_network_controller(int index);








