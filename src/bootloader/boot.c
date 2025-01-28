
/*
Currently I am using Limine Bootloader, so I will use the Limine Bootloader protocol 
to get the bootloader information. The bootloader information is stored in the 
bootloader_info_request struct. The bootloader_info_request struct is defined in 
the kernel.c file. The bootloader_info_request struct contains the bootloader name 
and version. The bootloader name and version are stored in the name and version  
fields of the bootloader_info_request struct.

In future I will implement a custom bootloader and will use that .

Reference: https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md#kernel-address-feature
*/

#include "../lib/string.h"      // memcpy
#include "../limine/limine.h"   // bootloader info
#include "../driver/vga.h"      // print
#include "../util/util.h"       //  print_size_with_units

#include "boot.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;




// Get Firmware info
__attribute__((used, section(".requests")))
static volatile struct limine_firmware_type_request firmware_type_request = {
    .id = LIMINE_FIRMWARE_TYPE_REQUEST,
    .revision = 3
};

char *FIRMWARE_TYPE;

void get_firmware_info(){
    if(firmware_type_request.response != NULL){
        uint64_t firmware_type = firmware_type_request.response->firmware_type;
        if(firmware_type == LIMINE_FIRMWARE_TYPE_X86BIOS){
            FIRMWARE_TYPE = "X86BIOS";
        }else if(firmware_type == LIMINE_FIRMWARE_TYPE_UEFI32){
            FIRMWARE_TYPE = "UEFI32";
        }else if(firmware_type == LIMINE_FIRMWARE_TYPE_UEFI64){
            FIRMWARE_TYPE = "UEFI64";
        } 
    }else{
        FIRMWARE_TYPE = "No firmware type found!";
        print("No firmware type found!\n");
    }
}

void print_firmware_info(){
    if(firmware_type_request.response != NULL){
        print("Firmware type: ");
        print(FIRMWARE_TYPE);
        print("\n");
    }else{
        FIRMWARE_TYPE = "No firmware type found!";
        print("No firmware type found!\n");
    }
}







// Get Paging info

__attribute__((used, section(".requests")))
static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 3
};


char *LIMINE_PAGING_MODE;

void get_paging_mode_info(){
    if(paging_mode_request.response != NULL){
        uint64_t mode = paging_mode_request.response->mode;
        if(mode == LIMINE_PAGING_MODE_X86_64_4LVL){
            LIMINE_PAGING_MODE = "Limine Paging mode x86_64 4 Level";
        } else if(mode == LIMINE_PAGING_MODE_X86_64_5LVL){
            LIMINE_PAGING_MODE = "Limine Paging mode x86_64 5 Level";
        }
    }else{
        LIMINE_PAGING_MODE = "No Paging mode found!";
        print("No Paging mode found!\n");
    }
}

void print_paging_mode_info(){
     if(paging_mode_request.response != NULL){
        print("Limine Paging Mode : ");
        print(LIMINE_PAGING_MODE);
        print("\n");
    }else{
        LIMINE_PAGING_MODE = "No Paging mode found!";
        print("No Paging mode found!\n");
    }
}


// Get  Limine Bootloader name, version etc 
__attribute__((used, section(".requests")))
static volatile struct limine_bootloader_info_request limine_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 3
};

char *BOOTLOADER_NAME;
char *BOOTLOADER_VERSION;

void get_limine_info(){
    if(limine_info_request.response != NULL){
        // uint64_t revision = limine_info_request.response->revision;
        BOOTLOADER_NAME = limine_info_request.response->name;
        BOOTLOADER_VERSION = limine_info_request.response->version;
    }else{
        BOOTLOADER_NAME = "No Limine Bootloader Info found!";
        BOOTLOADER_VERSION = NULL;
        print("No Limine Bootloader Info found!\n");
    }
}

void print_limine_info(){
    if(limine_info_request.response != NULL){
        print("Bootloader Name : ");
        print(BOOTLOADER_NAME);
        print("\n");
        print("Bootloader Version : ");
        print(BOOTLOADER_VERSION);
        print("\n");

    }else{
        print("No Limine Bootloader Info found!\n");
    }
}





// Get Stack size info
__attribute__((used, section(".requests")))
static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 3,
    .stack_size = 16384
};

uint64_t STACK_SIZE;

void get_stack_info(){
    if(stack_size_request.response != NULL){
        STACK_SIZE = stack_size_request.stack_size;
    }else{
        STACK_SIZE = 0;
        print("No stack size found!\n");
    }
}

void print_stack_info(){
    if(stack_size_request.response != NULL){
        print("Stack Size : ");
        print_dec(STACK_SIZE);
        print("\n");
    }else{
        print("No stack size found!\n");
    }
}



// Get Module info
__attribute__((used, section(".requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 3
};


void get_kernel_modules_info(){
    if(module_request.response != NULL){
        //uint64_t revision = module_request.response->revision;
        //uint64_t module_count = module_request.response->module_count;
        // struct limine_file **modules = module_request.response->modules;
    }else{
        print("No kernel modules found!\n");
    }
}

void print_kernel_modules_info(){
    if(module_request.response != NULL){
        // uint64_t revision = module_request.response->revision;
        uint64_t module_count = module_request.response->module_count;
        struct limine_file **modules = module_request.response->modules;
        for(size_t i=0;i<(size_t) module_count;i++){
            // uint64_t revision = modules[i]->revision;
            void *address = modules[i]->address;
            uint64_t size = modules[i]->size;
            char *path = modules[i]->path;
            // char *cmdline = modules[i]->cmdline;
            // uint32_t media_type = modules[i]->media_type;
            // uint32_t unused = modules[i]->unused;
            // uint32_t tftp_ip = modules[i]->tftp_ip;
            // uint32_t tftp_port = modules[i]->tftp_port;
            uint32_t partition_index = modules[i]->partition_index;
            uint32_t mbr_disk_id = modules[i]->mbr_disk_id;
            // struct limine_uuid gpt_disk_uuid = modules[i]->gpt_disk_uuid;
            // struct limine_uuid gpt_part_uuid = modules[i]->gpt_part_uuid;
            // struct limine_uuid part_uuid = modules[i]->part_uuid;

            print("Module Path : ");
            print(path);
            print("\n");

            print("Module Address : ");
            print_hex((uint64_t) address);
            print("\n");

            print("Module Size : ");
            print_size_with_units(size);
            print("\n");

            print("Module Media Type : ");
            // print_dec(media_type);
            print("\n");

            print("Module Partition Index : ");
            print_dec(partition_index);
            print("\n");

            print("Module MBR Disk ID : ");
            print_dec(mbr_disk_id);
            print("\n");
        }
    }else{
        print("No kernel modules found!\n");
    }
}



// finuls functions which will actually use in different ways
void get_bootloader_info(){
    get_kernel_modules_info();
    get_firmware_info();
    get_stack_info();
    get_limine_info();
    get_paging_mode_info();
}


void print_bootloader_info(void){
    print_kernel_modules_info();
    print_firmware_info();
    print_stack_info();
    print_limine_info();
    print_paging_mode_info();
}
