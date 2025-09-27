

#include "../../memory/kheap.h"

#include "../../lib/stdio.h"
#include "../../lib/string.h"

#include "controllers.h"


extern bool debug_on;

#define MAX_PCI_CONTROLLERS 256

pci_device_t* pci_devices;                  // Array to store detected PCI devices
int pci_devices_count = 0;                  // Counter for PCI devices

void alloc_controllers_memory(){

    pci_devices = (pci_device_t *) kheap_alloc(sizeof(pci_device_t) * MAX_PCI_CONTROLLERS, ALLOCATE_DATA);
    if(!pci_devices){
        printf("[Controllers] Failed to allocate memory for pci_devices\n");
        return;
    }
    memset((void*)pci_devices, 0, sizeof(pci_device_t) * MAX_PCI_CONTROLLERS);

    alloc_mass_storage_controllers_memory();
    alloc_network_controllers_memory();
    alloc_wireless_controllers_memory();
    alloc_display_controllers_memory();
    alloc_memory_controllers_memory();
    alloc_input_devices_memory();
    alloc_serial_bus_controllers_memory();
    alloc_bridge_devices_memory();
}

int get_pci_controllers_count(){
    return pci_devices_count;
}   

pci_device_t* get_pci_controller(int index){
    if(index < 0 || index >= pci_devices_count){
        return NULL;
    }
    return (pci_device_t*) &pci_devices[index];
}

void init_controllers(){
    printf("\n[Controllers] Initializing Controllers...\n");
    alloc_controllers_memory();

    pci_scan();

    if(!pci_devices){
        printf(" PCI Devices memory not allocated!\n");
        return;
    }

    if(pci_devices_count <= 0){
        printf(" No PCI devices found!\n");
        return;
    }

    for(int i = 0; i < pci_devices_count; i++){
        pci_device_t* device = get_pci_controller(i);
        if(!device){
            continue;
        }

        // Identify Mass Storage Controllers
        if(device->class_code == CONTROLLER_CLASS_MASS_STORAGE){
            if(debug_on) printf(" Mass Storage Controller found: Device ID: %x, Vendor ID: %x\n", device->device_id, device->vendor_id);
            mass_storage_controllers[mass_storage_controllers_count++] = *device;
        }

        // Identify Network Controllers
        if(device->class_code == CONTROLLER_CLASS_NETWORK){
            if(debug_on) printf(" Network Controller found: Device ID: %x, Vendor ID: %x\n", device->device_id, device->vendor_id);
            network_controllers[network_controllers_count++] = *device;
        }

        // Identify Wireless Controllers
        if(device->class_code == CONTROLLER_CLASS_WIRELESS){
            if(debug_on) printf(" Wireless Controller found: Device ID: %x, Vendor ID: %x\n", device->device_id, device->vendor_id);
            wireless_controllers[wireless_controllers_count++] = *device;
        }

        // Identify Display Controllers
        if(device->class_code == CONTROLLER_CLASS_DISPLAY){
            if(debug_on) printf(" Display Controller found: Device ID: %x, Vendor ID: %x\n", device->device_id, device->vendor_id);
            display_controllers[display_controllers_count++] = *device;
        }

        // Identify Memory Controllers
        if(device->class_code == CONTROLLER_CLASS_MEMORY){
            if(debug_on) printf(" Memory Controller found: Device ID: %x, Vendor ID: %x\n", device->device_id, device->vendor_id);
            memory_controllers[memory_controllers_count++] = *device;
        }

        // Identify Input Devices
        if(device->class_code == CONTROLLER_CLASS_INPUT_DEVICE){
            if(debug_on) printf(" Input Device Controller found: Device ID: %x, Vendor ID: %x\n", device->device_id, device->vendor_id);
            input_devices[input_devices_count++] = *device;
        }

        // Identify Serial Bus Controllers
        if(device->class_code == CONTROLLER_CLASS_SERIAL_BUS){
            if(debug_on) printf(" Serial Bus Controller found: Device ID: %x, Vendor ID: %x\n", device->device_id, device->vendor_id);
            serial_bus_controllers[serial_bus_controllers_count++] = *device;
        }
        
        // Identify Bridge Controllers
        if(device->class_code == CONTROLLER_CLASS_BRIDGE){
            if(debug_on) printf(" Bridge Controller found: Device ID: %x, Vendor ID: %x\n", device->device_id, device->vendor_id);
            bridge_devices[bridge_devices_count++] = *device;
        }
    }

    printf(" Total PCI Devices found: %d\n", get_pci_controllers_count());
    printf("[Controllers] Initialization Complete.\n\n");
}
