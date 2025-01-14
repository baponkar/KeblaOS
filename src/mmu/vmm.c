
/*
https://github.com/dreamportdev/Osdev-Notes/blob/master/04_Memory_Management/04_Virtual_Memory_Manager.md
https://chatgpt.com/share/675fa60d-c044-8001-aef6-d23b3d62ab62
*/

#include "vmm.h"

extern pml4_t *user_pml4;
extern pml4_t *kernel_pml4;
extern pml4_t *current_pml4;

#define VM_FLAG_NONE 0
#define VM_FLAG_WRITE (1 << 0)
#define VM_FLAG_EXEC (1 << 1)
#define VM_FLAG_USER (1 << 2)

#define PT_FLA_NX 0

extern uint64_t PHYSICAL_BASE;
extern uint64_t VIRTUAL_BASE;
extern uint64_t PHYSICAL_TO_VIRTUAL_OFFSET;

extern uint64_t HIGHER_HALF_DIRECT_MAP_OFFSET;

extern uint64_t KERNEL_MEM_START_ADDRESS;
extern uint64_t KERNEL_MEM_END_ADDRESS;
extern uint64_t KERNEL_MEM_LENGTH;

extern uint64_t USER_MEM_START_ADDRESS;
extern uint64_t USER_MEM_END_ADDRESS;
extern uint64_t USER_MEM_LENGTH;

uint64_t KERNEL_MEM_START_VIRT_ADDRESS;
uint64_t KERNEL_MEM_END_VIRT_ADDRESS;
uint64_t KERNEL_MEM_VIRT_LENGTH;

uint64_t USER_MEM_START_VIRT_ADDRESS;
uint64_t USER_MEM_END_VIRT_ADDRESS;
uint64_t USER_MEM_VIRT_LENGTH;



vm_object_t* vm_objs = NULL;


void init_vmm(){
    print("Start VMM initialization...\n");;
    KERNEL_MEM_START_VIRT_ADDRESS = phys_to_vir(KERNEL_MEM_START_ADDRESS);
    KERNEL_MEM_END_VIRT_ADDRESS = phys_to_vir(KERNEL_MEM_END_ADDRESS);
    KERNEL_MEM_VIRT_LENGTH = KERNEL_MEM_END_VIRT_ADDRESS - KERNEL_MEM_START_VIRT_ADDRESS;

    USER_MEM_START_VIRT_ADDRESS = phys_to_vir(USER_MEM_START_ADDRESS);
    USER_MEM_END_VIRT_ADDRESS = phys_to_vir(USER_MEM_END_ADDRESS);
    USER_MEM_VIRT_LENGTH = USER_MEM_END_VIRT_ADDRESS - USER_MEM_START_VIRT_ADDRESS;
    
    // print("KERNEL_MEM_START_VIRT_ADDRESS : ");
    // print_hex(KERNEL_MEM_START_VIRT_ADDRESS);
    // print("\n");

    // print("KERNEL_MEM_END_VIRT_ADDRESS : ");
    // print_hex(KERNEL_MEM_END_VIRT_ADDRESS);
    // print("\n");

    // print("KERNEL_MEM_VIRT_LENGTH : ");
    // print_hex(KERNEL_MEM_VIRT_LENGTH);
    // print("\n");

    // print("USER_MEM_START_VIRT_ADDRESS : ");
    // print_hex(USER_MEM_START_VIRT_ADDRESS);
    // print("\n");

    // print("USER_MEM_END_VIRT_ADDRESS : ");
    // print_hex(USER_MEM_END_VIRT_ADDRESS);
    // print("\n");

    // print("USER_MEM_VIRT_LENGTH : ");
    // print_hex(USER_MEM_VIRT_LENGTH);
    // print("\n");

    printf("Successfully initialized VMM.\n");
}


// Convert physical to virtual address
uint64_t phys_to_vir(uint64_t phy_addr){
    return phy_addr + PHYSICAL_TO_VIRTUAL_OFFSET;
}


// Convert virtual to physical address
uint64_t vir_to_phys(uint64_t vir_addr){
    return vir_addr - PHYSICAL_TO_VIRTUAL_OFFSET;
}


uint64_t convert_x86_64_vm_flags(size_t flags) {
    uint64_t ret_value = 0;
    if (flags & VM_FLAG_WRITE){
        ret_value |= PAGE_WRITE;
    }
        
    if (flags & VM_FLAG_USER){
        ret_value |= PAGE_USER;
    }
        
    if ((flags & VM_FLAG_EXEC) == 0){
        ret_value |= PT_FLA_NX;
    }
        
    return ret_value;
}


void* vmm_alloc(size_t length, size_t flags, void* arg){
    length = ((length + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE; // Page align

    vm_object_t* current = vm_objs;
    vm_object_t* prev = NULL;
    uintptr_t found = 0;

    while (current != NULL) {
        uintptr_t base = (prev == NULL ? 0 : prev->base + prev->length); // if prev = NULL then base = 0 otherwise base = prev->base + prev->length
        if (base + length <= current->base) {
            found = base;
            break;
        }

        prev = current;
        current = current->next;
    }

    if (found == 0) {
        found = (prev == NULL) ? KERNEL_MEM_START_VIRT_ADDRESS : prev->base + prev->length;
    }

    vm_object_t* new_obj = (vm_object_t*) kmalloc(sizeof(vm_object_t));
    new_obj->base = found;
    new_obj->length = length;
    new_obj->flags = flags;
    new_obj->next = current;

    if (prev != NULL) {
        prev->next = new_obj;
    } else {
        vm_objs = new_obj;
    }

    return (void*)found;
}


// This function will physical memory into 
void map_memory(void* _pml4, void* phys, void* virt, size_t flags){
    pml4_t *pml4 = (pml4_t *) _pml4;
    uint64_t pml4_index = PML4_INDEX((uint64_t)virt);
    uint64_t pdpt_index = PDPT_INDEX((uint64_t)virt);
    uint64_t pd_index = PD_INDEX((uint64_t)virt);
    uint64_t pt_index = PT_INDEX((uint64_t)virt);

    if (!pml4->entry_t[pml4_index].present) {
        pml4->entry_t[pml4_index].base_addr = (uint64_t)kmalloc(PAGE_SIZE) >> 12; // using physical address
        pml4->entry_t[pml4_index].present = 1;
        pml4->entry_t[pml4_index].rw = (flags & VM_FLAG_WRITE) ? 1 : 0;
    }

    pdpt_t* pdpt = (pdpt_t*)(pml4->entry_t[pml4_index].base_addr << 12);
    if (!pdpt->entry_t[pdpt_index].present) {
        pdpt->entry_t[pdpt_index].base_addr = (uint64_t)kmalloc(PAGE_SIZE) >> 12;
        pdpt->entry_t[pdpt_index].present = 1;
        pdpt->entry_t[pdpt_index].rw = (flags & VM_FLAG_WRITE) ? 1 : 0;
    }

    pd_t* pd = (pd_t*)(pdpt->entry_t[pdpt_index].base_addr << 12);
    if (!pd->entry_t[pd_index].present) {
        pd->entry_t[pd_index].base_addr = (uint64_t)kmalloc(PAGE_SIZE) >> 12;
        pd->entry_t[pd_index].present = 1;
        pd->entry_t[pd_index].rw = (flags & VM_FLAG_WRITE) ? 1 : 0;
    }

    pt_t* pt = (pt_t*)(pd->entry_t[pd_index].base_addr << 12);
    pt->pages[pt_index].frame = (uint64_t)phys >> 12;
    pt->pages[pt_index].present = 1;
    pt->pages[pt_index].rw = (flags & VM_FLAG_WRITE) ? 1 : 0;
}




void test_vmm() {

    print("\nAllocating 4096 bytes of memory...\n");
    void* allocated_memory = vmm_alloc(4096, VM_FLAG_WRITE | VM_FLAG_USER, NULL);
    if (allocated_memory) {
        print("Memory allocated at: ");
        print_hex((uint64_t) allocated_memory);
        print("\n");
    } else {
        print("Memory allocation failed!\n");
    }

    print("Mapping physical memory to virtual address...\n");

    map_memory(current_pml4, (void*)0x100000, allocated_memory, VM_FLAG_WRITE | VM_FLAG_USER);

    *(int*) allocated_memory = 25;

    print("Testing complete!\n");
}


