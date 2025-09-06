/*
Symmetrical Processor

References:
    https://github.com/limine-bootloader/limine/blob/v7.x/PROTOCOL.md
    https://wiki.osdev.org/Symmetric_Multiprocessing
*/


#include "../../lib/stdio.h"

#include "smp.h"


__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);

// Geting Symmetrical Processor Info
__attribute__((used, section(".requests")))
static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

struct limine_smp_response *smp_response;

uint32_t flags;
uint32_t bsp_lapic_id;
uint64_t cpu_count;
struct limine_smp_info **cpus;  // Pointer to an array of pointers to smp_info structures

// struct limine_smp_info {
//     uint32_t processor_id;
//     uint32_t lapic_id;
//     uint64_t reserved;
//     limine_goto_address goto_address;
//     uint64_t extra_argument;
// };

void get_smp_info(){
    smp_response = smp_request.response;
    if(smp_response == NULL){
        printf("[Error] SMP: smp_request not found!\n");
        return;
    }

    flags = smp_response->flags;
    bsp_lapic_id = smp_response->bsp_lapic_id;
    cpu_count = smp_response->cpu_count;
    cpus = smp_response->cpus;

    printf("[Info] SMP: Flags: %x, bsp_lapic_id: %d, cpu_count: %d\n", 
        flags, bsp_lapic_id, cpu_count, cpus);

    for(int i=0; i<cpu_count; i++){
        printf(" [-] cpu_id: %d, lapic_id: %d, reserved: %x, goto_address: %x, extra_argument: %x\n",
        cpus[i]->processor_id, cpus[i]->lapic_id, cpus[i]->reserved, (uint64_t)cpus[i]->goto_address, (uint64_t)cpus[i]->extra_argument);
    }
}










