
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
#include "../../../limine-8.6.0/limine.h"   // bootloader info
#include "../lib/stdio.h"      // print
#include "../util/util.h"       //  print_size_with_units
#include "firmware.h"

#include "boot.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;



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
        BOOTLOADER_NAME = "[Error] : No Limine Bootloader Info found!";
        BOOTLOADER_VERSION = NULL;
    }
}


void print_limine_info(){
    if(limine_info_request.response != NULL){

        printf("[Info] %s - %s bootloader.\n", BOOTLOADER_NAME, BOOTLOADER_VERSION);

    }else{
        printf("[Error] : No Limine Bootloader Info found!\n");
    }
}


// finuls functions which will actually use in different ways
void get_bootloader_info(){
    get_limine_info();
}


void print_bootloader_info(){
    print_limine_info();
}


