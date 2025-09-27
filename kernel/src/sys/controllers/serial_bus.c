

#include "../../memory/kheap.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../driver/pci/pci.h"

#include "serial_bus.h"

#define MAX_SERIAL_BUS_CONTROLLERS 32

pci_device_t *serial_bus_controllers;         // Array to store detected serial bus controllers
int serial_bus_controllers_count = 0;         // Counter for serial bus controllers

void alloc_serial_bus_controllers_memory(){
    serial_bus_controllers = (pci_device_t *) kheap_alloc(sizeof(pci_device_t) * MAX_SERIAL_BUS_CONTROLLERS, ALLOCATE_DATA);
    if(!serial_bus_controllers ){
        printf("[SERIAL BUS CONTROLLERS] Failed to allocate memory for serial_bus_controllers\n");
        return;
    }
    memset((void*)serial_bus_controllers, 0, sizeof(pci_device_t) * MAX_SERIAL_BUS_CONTROLLERS);
}

int get_serial_bus_controllers_count(){
    return serial_bus_controllers_count;
}

pci_device_t* get_serial_bus_controller(int index){
    if(index < 0 || index >= serial_bus_controllers_count){
        return NULL;
    }
    return (pci_device_t*) &serial_bus_controllers[index];
}

void detected_serial_bus_controller_info(){
    for(int i = 0; i < serial_bus_controllers_count; i++){
        pci_device_t *device = ( pci_device_t *) &serial_bus_controllers[i];
        
        if(device->subclass_code == 0x0){
            printf("[SERIAL BUS CONTROLLERS] Detected FireWire (IEEE 1394) Controller\n");
        } else if(device->subclass_code == 0x1){
            printf("[SERIAL BUS CONTROLLERS] Detected ACCESS Bus Controller\n");
        } else if(device->subclass_code == 0x2){
            printf("[SERIAL BUS CONTROLLERS] Detected SSA Controller\n");
        } else if(device->subclass_code == 0x3){
            printf("[SERIAL BUS CONTROLLERS] Detected USB Controller\n");
        } else if(device->subclass_code == 0x4){
            printf("[SERIAL BUS CONTROLLERS] Detected Fibre Channel Controller\n");
        } else if(device->subclass_code == 0x5){
            printf("[SERIAL BUS CONTROLLERS] Detected SMBus Controller\n");
        } else if(device->subclass_code == 0x6){
            printf("[SERIAL BUS CONTROLLERS] Detected InfiniBand Controller\n");
        } else if(device->subclass_code == 0x7){
            printf("[SERIAL BUS CONTROLLERS] Detected IPMI Interface Controller\n");
        } else if(device->subclass_code == 0x8){
            printf("[SERIAL BUS CONTROLLERS] Detected SERCOS Interface Controller\n");
        } else if(device->subclass_code == 0x9){
            printf("[SERIAL BUS CONTROLLERS] Detected CANbus Controller\n");
        } else {
            printf("[SERIAL BUS CONTROLLERS] Detected Other Serial Bus Controller\n");
        }
    }
}
















