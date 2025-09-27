#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


typedef enum {
    WIRELESS_SUBCLASS_IRDA = 0x0,
    WIRELESS_SUBCLASS_IR = 0x1,
    WIRELESS_SUBCLASS_RF = 0x2,
    WIRELESS_SUBCLASS_BLUETOOTH = 0x3,
    WIRELESS_SUBCLASS_BROADBAND = 0x4,
    WIRELESS_SUBCLASS_ETHERNET_802_1_a = 0x5,
    WIRELESS_SUBCLASS_ETHERNET_802_1_b = 0x6,
    WIRELESS_SUBCLASS_OTHER = 0x80
}WIRELESS_SUBCLASS_CODE;

extern pci_device_t *wireless_controllers;         // Array to store detected wireless controllers
extern int wireless_controllers_count;             // Counter for wireless controllers

void alloc_wireless_controllers_memory();
int get_wireless_controllers_count();
pci_device_t* get_wireless_controller(int index);








