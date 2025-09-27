
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "bridge_device.h"
#include "communication.h"
#include "data_acquisition_signal_processing.h"
#include "display.h"
#include "docking_stations.h"
#include "encryption_decryption.h"
#include "input_device.h"
#include "intelligent_io.h"
#include "legacy.h"
#include "mass_storage.h"
#include "memory.h"
#include "multimedia.h"
#include "network.h"
#include "processors.h"
#include "satellite_communication.h"
#include "serial_bus.h"
#include "system_peripherals.h"
#include "wireless.h"

#include "../../driver/pci/pci.h"




typedef enum {
    CONTROLLER_CLASS_UNCLASSIFIED = 0x0,
    CONTROLLER_CLASS_MASS_STORAGE = 0x1,
    CONTROLLER_CLASS_NETWORK = 0x2,
    CONTROLLER_CLASS_DISPLAY = 0x3,
    CONTROLLER_CLASS_MULTIMEDIA = 0x4,
    CONTROLLER_CLASS_MEMORY = 0x5,
    CONTROLLER_CLASS_BRIDGE = 0x6,
    CONTROLLER_CLASS_SIMPLE_COMMUNICATION = 0x7,
    CONTROLLER_CLASS_BASE_SYSTEM_PERIPHERAL = 0x8,
    CONTROLLER_CLASS_INPUT_DEVICE = 0x9,
    CONTROLLER_CLASS_DOCKING_STATION = 0xA,
    CONTROLLER_CLASS_PROCESSOR = 0xB,
    CONTROLLER_CLASS_SERIAL_BUS = 0xC,
    CONTROLLER_CLASS_WIRELESS = 0xD,
    CONTROLLER_CLASS_INTELLIGENT = 0xE,
    CONTROLLER_CLASS_SATELLITE_COMMUNICATION = 0xF,
    CONTROLLER_CLASS_ENCRYPTION = 0x10,
    CONTROLLER_CLASS_SIGNAL_PROCESSING = 0x11,
    CONTROLLER_CLASS_PROCESSING_ACCELERATOR = 0x12,
    CONTROLLER_CLASS_NON_ESSENTIAL_INSTRUMENTATION = 0x13,
    CONTROLLER_CLASS_RESERVED_1 = 0x14,             // 0x14 - 0x3F (Reserved)
    CONTROLLER_CLASS_COPROCESSOR = 0x40,
    CONTROLLER_CLASS_RESERVED_2 = 0x41,             // 0x41 - 0xFE (Reserved)
    CONTROLLER_CLASS_UNASSIGNED = 0xFF
}CONTROLLERS_CLASS_CODE;

extern pci_device_t* pci_devices;                   // Array to store detected PCI devices
extern int pci_devices_count;                       // Counter for PCI devices

void alloc_controllers_memory();
int get_pci_controllers_count();
pci_device_t* get_pci_controller(int index);


void init_controllers();



