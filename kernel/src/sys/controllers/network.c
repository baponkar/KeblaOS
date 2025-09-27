
#include "../../memory/kheap.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../driver/pci/pci.h"

#include "network.h"



#define NETWORK_CLASS_CODE 0x02

#define NETWORK_SUBCLASS_ETHERNET 0x00
#define NETWORK_SUBCLASS_TOKEN_RING 0x01
#define NETWORK_SUBCLASS_FDDI 0x02
#define NETWORK_SUBCLASS_ATM 0x03
#define NETWORK_SUBCLASS_ISDN 0x04
#define NETWORK_SUBCLASS_WORLDFIP 0x05
#define NETWORK_SUBCLASS_PICMG 0x06
#define NETWORK_SUBCLASS_INFINIBAND 0x07
#define NETWORK_SUBCLASS_FABRIC 0x08
#define NETWORK_SUBCLASS_OTHER 0x80

#define MAX_NETWORK_CONTROLLERS 32


pci_device_t *network_controllers;          // Array to store detected network controllers
int network_controllers_count = 0;          // Counter for network controllers



void alloc_network_controllers_memory(){
    network_controllers = (pci_device_t *) kheap_alloc(sizeof(pci_device_t) * MAX_NETWORK_CONTROLLERS, ALLOCATE_DATA);
    if(!network_controllers ){
        printf("[NETWORK CONTROLLERS] Failed to allocate memory for network_controllers\n");
        return;
    }
    memset((void*)network_controllers, 0, sizeof(pci_device_t) * MAX_NETWORK_CONTROLLERS);
}


int get_network_controllers_count(){
    return network_controllers_count;
}

pci_device_t* get_network_controller(int index){
    if(index < 0 || index >= network_controllers_count){
        return NULL;
    }
    return (pci_device_t*) &network_controllers[index];
}

