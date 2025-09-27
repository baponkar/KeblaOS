
#include "../../memory/kheap.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../driver/pci/pci.h"

#include "mass_storage.h"


#define MAX_MASS_STORAGE_CONTROLLERS 32


pci_device_t* mass_storage_controllers;     // Array to store detected mass storage devices
int mass_storage_controllers_count = 0;     // Counter for mass storage devices


void alloc_mass_storage_controllers_memory(){
    mass_storage_controllers = (pci_device_t *) kheap_alloc(sizeof(pci_device_t) * MAX_MASS_STORAGE_CONTROLLERS, ALLOCATE_DATA);
    if(!mass_storage_controllers ){
        printf("[MASS STORAGE CONTROLLERS] Failed to allocate memory for mass_storage_controllers\n");
        return;
    }
    memset((void*)mass_storage_controllers, 0, sizeof(pci_device_t) * MAX_MASS_STORAGE_CONTROLLERS);
}


int get_mass_storage_controllers_count(){
    return mass_storage_controllers_count;
}

pci_device_t* get_mass_storage_controller(int index){
    if(index < 0 || index >= mass_storage_controllers_count){
        return NULL;
    }
    return (pci_device_t*) &mass_storage_controllers[index];
}



