/*


*/

#include "../lib/stdio.h"
#include "../../../limine-9.2.3/limine.h"
#include "../lib/string.h"
#include "../bootloader/boot.h"
#include "../driver/io/ports.h"
#include "../sys/ahci/ahci.h"


#include "sysinfo.h"



__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

static volatile struct limine_smbios_request smbios_request = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0
};

static volatile struct limine_efi_system_table_request efi_system_table_request = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 0
};

static volatile struct limine_efi_memmap_request efi_memmap_request = {
    .id = LIMINE_EFI_MEMMAP_REQUEST,
    .revision = 0
};

void get_smbios_info(){
    if(!smbios_request.response){
        printf("[Error] smbios_request.response is null\n");
        return;
    }
    uint64_t revision = smbios_request.response->revision;
    void *entry_32 = smbios_request.response->entry_32;
    void *entry_64 = smbios_request.response->entry_64;
}


void get_efi_system_table_info(){
    if(!efi_system_table_request.response){
        printf("[Error] efi_system_table_request.response is null\n");
        return;
    }

    uint64_t revision = efi_system_table_request.response->revision;
    void *address = efi_system_table_request.response->address;
}

void get_efi_memmap_info(){
    if(!efi_memmap_request.response){
        printf("[Error] efi_efi_memmap_request.response is null\n");
        return;
    }
    uint64_t revision = efi_memmap_request.response->revision;
    void *memmap = efi_memmap_request.response->memmap;
    uint64_t memmap_size = efi_memmap_request.response->memmap_size;
    uint64_t desc_size = efi_memmap_request.response->desc_size;
    uint64_t desc_version = efi_memmap_request.response->desc_version;
}