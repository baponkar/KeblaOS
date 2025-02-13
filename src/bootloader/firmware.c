
#include "../limine/limine.h"
#include "../lib/stdio.h"  


#include "firmware.h"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

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
        printf("Firmware type: %s\n", FIRMWARE_TYPE);
    }
}

void print_firmware_info(){
    if(firmware_type_request.response != NULL){
        printf("Firmware type: %s\n", FIRMWARE_TYPE);
    }else{
        FIRMWARE_TYPE = "No firmware type found!";
        printf("%s\n", FIRMWARE_TYPE);
    }
}