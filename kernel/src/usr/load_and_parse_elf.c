
/*
Loading ELF File 

References:
    https://wiki.osdev.org/ELF
*/


#include "../../../limine-9.2.3/limine.h"
#include "../lib/stdio.h"
#include "../lib/string.h"

#include "../util/util.h"

#include "../memory/Uheap.h"

#include "load_and_parse_elf.h"



// Get Module info by using limine bootloader
__attribute__((used, section(".requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

extern __attribute__((naked, noreturn)) void switch_to_user_mode(uint64_t stack_addr, uint64_t code_addr);

void get_kernel_modules_info(){
    if(module_request.response != NULL){
        uint64_t revision = module_request.response->revision;
        uint64_t module_count = module_request.response->module_count;
        struct limine_file **modules = module_request.response->modules;
    }else{
        printf("[Error] No kernel modules found!\n");
    }
}

void print_kernel_modules_info(){
    if(module_request.response != NULL){
        uint64_t revision = module_request.response->revision;
        uint64_t module_count = module_request.response->module_count;
        struct limine_file **modules = module_request.response->modules;
        for(size_t i=0;i<(size_t) module_count;i++){
            uint64_t revision = modules[i]->revision;
            void *address = modules[i]->address;
            uint64_t size = modules[i]->size;
            char *path = modules[i]->path;
            char *cmdline = modules[i]->cmdline;
            uint32_t media_type = modules[i]->media_type;
            uint32_t unused = modules[i]->unused;
            uint32_t tftp_ip = modules[i]->tftp_ip;
            uint32_t tftp_port = modules[i]->tftp_port;
            uint32_t partition_index = modules[i]->partition_index;
            uint32_t mbr_disk_id = modules[i]->mbr_disk_id;
            struct limine_uuid gpt_disk_uuid = modules[i]->gpt_disk_uuid;
            struct limine_uuid gpt_part_uuid = modules[i]->gpt_part_uuid;
            struct limine_uuid part_uuid = modules[i]->part_uuid;

            printf("Module Path : %s\n", path);
            printf("Module Address : %x\n", (uint64_t) address);
            printf("Module Size : ");
            print_size_with_units(size);
            printf("\n");

            printf("Module Media Type : %d\n", media_type);

            printf("Module Partition Index : %d\n", partition_index);

            printf("Module MBR Disk ID : %d\n", mbr_disk_id);
        }
    }else{
        printf("[Error] No kernel modules found!\n");
    }
}



void load_user_elf_and_jump() {

    void *elf_base = module_request.response->modules[0]->address;

    printf("user_program.elf base addr %x\n", (uint64_t) elf_base);

    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_base;
    Elf64_Phdr *phdrs = (Elf64_Phdr *)((uint8_t *)elf_base + ehdr->e_phoff);

    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *ph = &phdrs[i];
        if (ph->p_type != PT_LOAD) continue;

        void *src = (uint8_t *)elf_base + ph->p_offset;
        void *dst = (void *)ph->p_vaddr;

        memcpy(dst, src, ph->p_filesz);
        memset((uint8_t *)dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
    }

    uint64_t user_entry = ehdr->e_entry;
    uint64_t user_stack = (uint64_t) uheap_alloc(0x4000); 
    
    printf("Switching into usermode: user_entry_addr-%x, user_stack_addr-%x\n",
        user_entry, user_stack);

    switch_to_user_mode(user_stack, user_entry);
}


