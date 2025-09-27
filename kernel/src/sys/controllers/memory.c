#include "../../memory/kheap.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../driver/pci/pci.h"

#include "memory.h"

#define MAX_MEMORY_CONTROLLERS 32

pci_device_t *memory_controllers;         // Array to store detected memory controllers
int memory_controllers_count = 0;         // Counter for memory controllers

void alloc_memory_controllers_memory(){
    memory_controllers = (pci_device_t *) kheap_alloc(sizeof(pci_device_t) * MAX_MEMORY_CONTROLLERS, ALLOCATE_DATA);
    if(!memory_controllers ){
        printf("[MEMORY CONTROLLERS] Failed to allocate memory for memory_controllers\n");
        return;
    }
    memset((void*)memory_controllers, 0, sizeof(pci_device_t) * MAX_MEMORY_CONTROLLERS);
}

int get_memory_controllers_count(){
    return memory_controllers_count;
}

pci_device_t* get_memory_controller(int index){
    if(index < 0 || index >= memory_controllers_count){
        return NULL;
    }
    return (pci_device_t*) &memory_controllers[index];
}


