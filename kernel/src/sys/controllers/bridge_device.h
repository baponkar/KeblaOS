#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#include "../driver/pci/pci.h"


typedef enum {
    BRIDGE_DEVICE_SUBCLASS_HOST = 0x0,
    BRIDGE_DEVICE_SUBCLASS_ISA = 0x1,
    BRIDGE_DEVICE_SUBCLASS_EISA = 0x2,
    BRIDGE_DEVICE_SUBCLASS_MCA = 0x3,
    BRIDGE_DEVICE_SUBCLASS_PCI_TO_PCI_1 = 0x4,
    BRIDGE_DEVICE_SUBCLASS_PCMCIA = 0x5,
    BRIDGE_DEVICE_SUBCLASS_NUBUS = 0x6,
    BRIDGE_DEVICE_SUBCLASS_CARD_BUS = 0x7,
    BRIDGE_DEVICE_SUBCLASS_RACE_WAY = 0x8,
    BRIDGE_DEVICE_SUBCLASS_PCI_TO_PCI_2 = 0x9,
    BRIDGE_DEVICE_SUBCLASS_INFINI_BAND_TO_PCI = 0xA,
    BRIDGE_DEVICE_SUBCLASS_OTHER = 0x80
}BRIDGE_DEVICE_SUBCLASS_CODE;

extern pci_device_t *bridge_devices;         // Array to store detected bridge devices
extern int bridge_devices_count;             // Counter for bridge devices

void alloc_bridge_devices_memory();
int get_bridge_devices_count();
pci_device_t* get_bridge_device(int index);


