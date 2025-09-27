
#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../memory/kheap.h"


#include "display.h"





#define MAX_DISPLAY_CONTROLLERS 32

pci_device_t *display_controllers;         // Array to store detected display controllers
int display_controllers_count = 0;         // Counter for display controllers

void alloc_display_controllers_memory(){
    display_controllers = (pci_device_t *) kheap_alloc(sizeof(pci_device_t) * MAX_DISPLAY_CONTROLLERS, ALLOCATE_DATA);
    if(!display_controllers ){
        printf("[DISPLAY CONTROLLERS] Failed to allocate memory for display_controllers\n");
        return;
    }
    memset((void*)display_controllers, 0, sizeof(pci_device_t) * MAX_DISPLAY_CONTROLLERS);
}

int get_display_controllers_count(){
    return display_controllers_count;
}

pci_device_t* get_display_controller(int index){
    if(index < 0 || index >= display_controllers_count){
        return NULL;
    }
    return (pci_device_t*) &display_controllers[index];
}



