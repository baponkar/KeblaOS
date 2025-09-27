
#include "../../memory/kheap.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../driver/pci/pci.h"

#include "wireless.h"



#define MAX_WIRELESS_CONTROLLERS 32

pci_device_t *wireless_controllers;         // Array to store detected wireless controllers
int wireless_controllers_count = 0;         // Counter for wireless controllers

void alloc_wireless_controllers_memory(){
    wireless_controllers = (pci_device_t *) kheap_alloc(sizeof(pci_device_t) * MAX_WIRELESS_CONTROLLERS, ALLOCATE_DATA);
    if(!wireless_controllers ){
        printf("[WIRELESS CONTROLLERS] Failed to allocate memory for wireless_controllers\n");
        return;
    }
    memset((void*)wireless_controllers, 0, sizeof(pci_device_t) * MAX_WIRELESS_CONTROLLERS);
}

int get_wireless_controllers_count(){
    return wireless_controllers_count;
}

pci_device_t* get_wireless_controller(int index){
    if(index < 0 || index >= wireless_controllers_count){
        return NULL;
    }
    return (pci_device_t*) &wireless_controllers[index];
}



