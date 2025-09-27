
#include "../../memory/kheap.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../driver/pci/pci.h"

#include "input_device.h"

#define MAX_INPUT_DEVICES 32
pci_device_t *input_devices;         // Array to store detected input devices
int input_devices_count = 0;         // Counter for input devices

void alloc_input_devices_memory(){
    input_devices = (pci_device_t *) kheap_alloc(sizeof(pci_device_t) * MAX_INPUT_DEVICES, ALLOCATE_DATA);
    if(!input_devices ){
        printf("[INPUT DEVICES] Failed to allocate memory for input_devices\n");
        return;
    }
    memset((void*)input_devices, 0, sizeof(pci_device_t) * MAX_INPUT_DEVICES);
}

int get_input_devices_count(){
    return input_devices_count;
}

pci_device_t* get_input_device(int index){
    if(index < 0 || index >= input_devices_count){
        return NULL;
    }
    return (pci_device_t*) &input_devices[index];
}

void detected_input_device_info(){
    for(int i = 0; i < input_devices_count; i++){
        pci_device_t *device = ( pci_device_t *) &input_devices[i];
        
        if(device->subclass_code == INPUT_DEVICE_SUBCLASS_KEYBOARD){
            printf("[INPUT DEVICES] Detected Keyboard Controller\n");
        } else if(device->subclass_code == INPUT_DEVICE_SUBCLASS_MOUSE){
            printf("[INPUT DEVICES] Detected Mouse Controller\n");
        } else if(device->subclass_code == INPUT_DEVICE_SUBCLASS_DIGITIZER_PEN){
            printf("[INPUT DEVICES] Detected Digitizer Pen Controller\n");
        } else if(device->subclass_code == INPUT_DEVICE_SUBCLASS_SCANNER){
            printf("[INPUT DEVICES] Detected Scanner Controller\n");
        } else if(device->subclass_code == INPUT_DEVICE_SUBCLASS_GAMEPORT){
            printf("[INPUT DEVICES] Detected Gameport Controller\n");
        } else {
            printf("[INPUT DEVICES] Detected Other Input Device\n");
        }
    }
}