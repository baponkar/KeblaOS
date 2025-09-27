
#include "../../memory/kheap.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../driver/pci/pci.h"

#include "bridge_device.h"



#define MAX_BRIDGE_DEVICES 32

pci_device_t *bridge_devices;         // Array to store detected bridge devices
int bridge_devices_count = 0;         // Counter for bridge devices

void alloc_bridge_devices_memory(){
    bridge_devices = (pci_device_t *) kheap_alloc(sizeof(pci_device_t) * MAX_BRIDGE_DEVICES, ALLOCATE_DATA);
    if(!bridge_devices ){
        printf("[BRIDGE DEVICES] Failed to allocate memory for bridge_devices\n");
        return;
    }
    memset((void*)bridge_devices, 0, sizeof(pci_device_t) * MAX_BRIDGE_DEVICES);
}

int get_bridge_devices_count(){
    return bridge_devices_count;
}

pci_device_t* get_bridge_device(int index){
    if(index < 0 || index >= bridge_devices_count){
        return NULL;
    }
    return (pci_device_t*) &bridge_devices[index];
}









