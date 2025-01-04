/*
    This will implement the kernel heap. 
    The kernel heap is a simple memory allocator that is used to allocate memory dynamically. 

    https://web.archive.org/web/20160326122206/http://jamesmolloy.co.uk/tutorial_html/7.-The%20Heap.html


*/


#include "kheap.h"

extern uint64_t PHYSICAL_TO_VIRTUAL_OFFSET;

heap_t *kheap;



// Low level memory allocation by usin base as KERNEL_MEM_START_ADDRESS
uint64_t kmalloc(uint64_t sz)       // vanilla (normal).
{
    uint64_t tmp = (uint64_t) KERNEL_MEM_START_ADDRESS; // memory allocate in current placement address
    KERNEL_MEM_START_ADDRESS += sz;    // increase the placement address for next memory allocation
    return tmp;
}


/*
    Page Alignment: Page alignment ensures that memory addresses fall on boundaries that are 
    multiples of the page size (typically 4 KB for 64-bit systems).
*/

uint64_t kmalloc_a(uint64_t sz, int align)    // page aligned.
{
    /*
    page directory and page table addresses need to be page-aligned: that is, the bottom 12 
    bits need to be zero (otherwise they would interfere with the read/write/protection/accessed bits).
    */
    if (align == 1 && (KERNEL_MEM_START_ADDRESS & 0xFFF)) // If the address is not already page-aligned i.e. multiple of PAGE_SIZE
    {
        // Align it.
        KERNEL_MEM_START_ADDRESS &= 0xFFFFFFFFFFFFF000; // masking of most significant 20 bit which is used for address
        KERNEL_MEM_START_ADDRESS += FRAME_SIZE;    // increase  the placement address by 4 KB, Page Size
    }
    
    uint64_t tmp = KERNEL_MEM_START_ADDRESS;
    KERNEL_MEM_START_ADDRESS += sz;

    return tmp;
}


/*
The kmalloc_p function is used to allocate memory while also optionally returning the physical address 
of the allocated memory.
*/
uint64_t kmalloc_p(uint64_t sz, uint64_t *phys){
    if (phys)
    {   // phys (parameter): This is a pointer to a uint64_t variable where 
        // the physical address of the allocated memory will be stored.
        *phys = KERNEL_MEM_START_ADDRESS;
    }
    uint64_t tmp = KERNEL_MEM_START_ADDRESS;
    KERNEL_MEM_START_ADDRESS += sz;
    return tmp;
}


/*
The kmalloc_ap function is used to allocate memory , returning the physical address,  
and also ensure page boundary alignment
*/
uint64_t kmalloc_ap(uint64_t sz, int align, uint64_t *phys)  // page aligned and returns a physical address.
{
    if (align == 1 && (KERNEL_MEM_START_ADDRESS & 0xFFF)) // If the address is not already page-aligned and want to make it page aligned
    {
        // Align it.
        KERNEL_MEM_START_ADDRESS &= 0xFFFFFFFFFFFFF000;
        KERNEL_MEM_START_ADDRESS += PAGE_SIZE;    // increase  the placement address by 4 KB, Page Size
    }
    if (phys)
    {
        *phys = KERNEL_MEM_START_ADDRESS;
    }
    uint64_t tmp = KERNEL_MEM_START_ADDRESS;
    KERNEL_MEM_START_ADDRESS += sz;
    return tmp;
}

/* Dynamic Memory allocation by jamesmolloy.co.uk*/


#define KHEAP_INITIAL_SIZE  0x100000    // 1 MB
#define HEAP_INDEX_SIZE     0x20000     // 128 KB
#define HEAP_MAGIC          0x123890AB  // Magic number for error checking and identification.
#define HEAP_MIN_SIZE       0x4000     // 16 KB


void init_kheap()
{
    uint64_t start_addr = PAGE_ALIGN(KERNEL_MEM_START_ADDRESS);
    uint64_t end_addr = PAGE_ALIGN(KERNEL_MEM_START_ADDRESS + 2*KHEAP_INITIAL_SIZE);
    uint64_t max_addr = PAGE_ALIGN(KERNEL_MEM_START_ADDRESS + KERNEL_MEM_LENGTH);

    
    uint64_t start_virtual_addr = start_addr + PHYSICAL_TO_VIRTUAL_OFFSET;
    uint64_t end_virtual_addr = end_addr + PHYSICAL_TO_VIRTUAL_OFFSET;

    // Now allocate those pages we mapped earlier.
    for (uint64_t i = start_virtual_addr; i < end_virtual_addr; i += PAGE_SIZE){

        // print("virtual address: ");
        // print_hex(i);
        // print("\n");
        // page_t *page = get_page(i, 1, current_pml4);

        // print("page: ");
        // print_hex((uint64_t)page);
        // print("\n");
        alloc_frame( get_page(i, 1, current_pml4), 0, 0);

    }



    kheap = create_heap(start_addr, end_addr, max_addr, 0, 0);  // supervisor = 0, readonly = 0

    print("kheap pointer: ");
    print_hex((uint64_t)kheap);
    print("\n");
    
    print("Kernel Heap initialized successfully!\n");
}


// The below function will return 1 if a < b, else 0.
int8_t standard_lessthan_predicate(type_t a, type_t b)
{
   return (a<b) ? 1:0;
}


// Create an ordered array.
ordered_array_t create_ordered_array(uint64_t max_size, lessthan_predicate_t less_than)
{
   ordered_array_t to_ret;
   to_ret.array = (void*) kmalloc(max_size * sizeof(type_t));
   memset(to_ret.array, 0, max_size * sizeof(type_t));
   to_ret.size = 0;
   to_ret.max_size = max_size;
   to_ret.less_than = less_than; // assign the function pointer to the less_than function.
   return to_ret;
}



// Place the ordered array at a certain address.
ordered_array_t place_ordered_array(void *addr, uint64_t max_size, lessthan_predicate_t less_than)
{
   ordered_array_t to_ret;
   to_ret.array = (type_t*) addr;
   memset(to_ret.array, 0, max_size * sizeof(type_t)); // for  safety purpose clearing all the memory.
   to_ret.size = 0; // initially size is 0
   to_ret.max_size = max_size;
   to_ret.less_than = less_than;
   return to_ret;
}


// Destroy an ordered array.
void destroy_ordered_array(ordered_array_t *array)
{
    // kfree(array->array);
}


void insert_ordered_array(type_t item, ordered_array_t *array) {
    // Check if less_than function pointer is valid
    assert(array->less_than);

    // Check if there is space to insert the item
    assert(array->size < array->max_size);

    // Find the correct index for the item
    uint64_t iterator = 0;
    while (iterator < array->size && array->less_than(array->array[iterator], item)) {
        iterator++;
    }

    // Shift elements to the right to make space for the new item
    for (uint64_t i = array->size; i > iterator; i--) {
        array->array[i] = array->array[i - 1];
    }

    // Insert the new item at the correct position
    array->array[iterator] = item;

    // Increment the size of the array
    array->size++;
}




// Lookup the item at index i.
type_t lookup_ordered_array(uint64_t i, ordered_array_t *array)
{
   assert(i < array->size);
   return array->array[i];
}




// Deletes the item at location i from the array.
void remove_ordered_array(uint64_t i, ordered_array_t *array)
{
   while (i < array->size)
   {
       array->array[i] = array->array[i+1];
       i++;
   }
   array->size--;
}



static int64_t find_smallest_hole(uint64_t size, uint8_t page_align, heap_t *heap) {
    // Iterate through the ordered array to find the smallest suitable hole
    uint64_t iterator = 0;
    while (iterator < heap->ordered_array.size) {
        // Retrieve the header from the ordered array
        header_t *header = (header_t *)lookup_ordered_array(iterator, &heap->ordered_array);

        // Check if this block is a hole
        if (header->is_hole == 0) {
            iterator++;
            continue;
        }

        // If page alignment is requested
        if (page_align > 0) {
            uint64_t location = (uint64_t)header;
            uint64_t offset = 0;

            // Calculate the alignment offset
            if (((location + sizeof(header_t)) & 0xFFFFFFFFFFFFF000) != 0) {
                offset = (PAGE_SIZE - ((location + sizeof(header_t)) % PAGE_SIZE)) % PAGE_SIZE;
            }

            // Calculate the effective hole size after alignment
            int64_t hole_size = (int64_t)header->size - offset;

            // Check if the hole can fit the requested size
            if (hole_size >= (int64_t)size) {
                break;
            }
        } else if (header->size >= size) {
            // If no alignment is required, check the hole size
            break;
        }

        iterator++;
    }

    // If we reached the end of the array without finding a suitable hole
    if (iterator == heap->ordered_array.size) {
        return -1; // No suitable hole found
    }

    // Return the index of the found hole
    return iterator;
}




static int8_t header_t_less_than(void*a, void *b)
{
   return (((header_t*)a)->size < ((header_t*)b)->size) ? 1:0;
}




static void expand(uint64_t new_size, heap_t *heap)
{
   // Sanity check new_size is greater than heap->end_address - heap->start_address.
   assert(new_size > (heap->end_address - heap->start_address) );

   // Get the nearest following page boundary.
   if (new_size & 0xFFFFFFFFFFFFF000 != 0){
       new_size &= 0xFFFFFFFFFFFFF000;
       new_size += PAGE_SIZE;
   }
   // Make sure we are not overreaching ourselves.
   assert( (heap->start_address + new_size) <= heap->max_address);

   // This should always be on a page boundary.
   uint64_t old_size = heap->end_address - heap->start_address;
   uint64_t i = old_size;
   while (i < new_size){
       alloc_frame( get_page(heap->start_address + i, 1, kernel_pml4), (heap->supervisor)?1:0, (heap->readonly)?0:1);
       i += PAGE_SIZE; 
   }
   heap->end_address = heap->start_address + new_size;
}


static uint64_t contract(uint64_t new_size, heap_t *heap)
{
   // Sanity check.
   assert(new_size < heap->end_address-heap->start_address);

   // Get the nearest following page boundary.
   if (new_size & PAGE_SIZE){
       new_size &= PAGE_SIZE; 
       new_size += PAGE_SIZE; 
   }

   // Don't contract too far!
   if (new_size < HEAP_MIN_SIZE){
       new_size = HEAP_MIN_SIZE;
   }

   uint64_t old_size = heap->end_address-heap->start_address;
   uint64_t i = old_size - PAGE_SIZE;

   while (new_size < i){
       free_frame(get_page(heap->start_address + i, 0, kernel_pml4));
       i -= PAGE_SIZE;
   }

   heap->end_address = heap->start_address + new_size;
   return new_size;
}


heap_t *create_heap(uint64_t start_addr, uint64_t end_addr, uint64_t max_addr, uint8_t supervisor, uint8_t readonly) {
    // Ensure valid memory range
    if (end_addr <= start_addr || max_addr < end_addr) {
        return NULL; // Invalid heap range
    }

    // Allocate memory for the heap structure
    heap_t *heap = (heap_t*) kmalloc_a(sizeof(heap_t), 1);

    if (!heap) {
        return NULL; // Allocation failed
    }

    // Ensure start and end addresses are page-aligned
    assert(start_addr % PAGE_SIZE == 0);
    assert(end_addr % PAGE_SIZE == 0);

    // Initialize the ordered array at the start address
    heap->ordered_array = place_ordered_array( (void*)start_addr, HEAP_INDEX_SIZE, &header_t_less_than);

    // Adjust the start address forward to where we can start putting data
    start_addr += sizeof(type_t) * HEAP_INDEX_SIZE;

    print("start address: ");
    print_hex(start_addr);
    print("\n");

    print("end address: ");
    print_hex(end_addr);
    print("\n");

    // Ensure the start address is page-aligned
    if ((start_addr & 0xFFFFFFFFFFFFF000) != 0) {
        start_addr = (start_addr & 0xFFFFFFFFFFFFF000) + PAGE_SIZE;
    }

    // Verify that the adjusted start address is still within bounds
    if (start_addr >= end_addr) {
        print("Not enough space to create a heap\n");
        return NULL; // Not enough space to create a heap
    }

    // Initialize heap structure
    heap->start_address = start_addr;
    heap->end_address = end_addr;
    heap->max_address = max_addr;
    heap->supervisor = supervisor;
    heap->readonly = readonly;

    // Create an initial large hole
    header_t *hole = (header_t *)start_addr;
    hole->size = end_addr - start_addr;
    hole->magic = HEAP_MAGIC;
    hole->is_hole = 1;

    print("hole: ");
    print_hex((uint64_t) hole);
    print("\n");

    // Insert the hole into the ordered array
    insert_ordered_array((void*)hole, &heap->ordered_array);

    return heap;
}



void *alloc(uint64_t size, uint8_t page_align, heap_t *heap) {
    // Ensure the size is non-zero
    if (size == 0) {
        return NULL;
    }

    // Find the smallest hole that fits the requested size
    int64_t hole_index = find_smallest_hole(size, page_align, heap);
    if (hole_index == -1) {
        return NULL; // No suitable hole found
    }

    // Retrieve the hole header from the ordered array
    header_t *hole_header = (header_t *)lookup_ordered_array(hole_index, &heap->ordered_array);

    // Calculate alignment offset if page alignment is requested
    uint64_t original_location = (uint64_t)hole_header;
    uint64_t offset = 0;
    if (page_align > 0) {
        uint64_t aligned_location = (original_location + sizeof(header_t) + 0xFFF) & ~0xFFF;
        offset = aligned_location - original_location - sizeof(header_t);
    }

    // Ensure the hole can accommodate the alignment and requested size
    if ((hole_header->size - offset) < size) {
        return NULL; // Should not happen, as find_smallest_hole should ensure this
    }

    // Split the hole if necessary
    if ((hole_header->size - offset) > (size + sizeof(header_t) + sizeof(footer_t))) {
        // Create a new header for the remaining part of the hole
        header_t *new_hole = (header_t *)((uint64_t)hole_header + sizeof(header_t) + size + offset + sizeof(footer_t));
        new_hole->size = hole_header->size - size - offset - sizeof(header_t) - sizeof(footer_t);
        new_hole->magic = HEAP_MAGIC;
        new_hole->is_hole = 1;

        // Update the footer of the new hole
        footer_t *new_footer = (footer_t *)((uint64_t)new_hole + new_hole->size - sizeof(footer_t));
        new_footer->magic = HEAP_MAGIC;
        new_footer->header = new_hole;

        // Update the hole size
        hole_header->size = size + offset;

        // Update the footer of the allocated block
        footer_t *allocated_footer = (footer_t *)((uint64_t)hole_header + sizeof(header_t) + size + offset - sizeof(footer_t));
        allocated_footer->magic = HEAP_MAGIC;
        allocated_footer->header = hole_header;

        // Insert the new hole into the ordered array
        insert_ordered_array((void *)new_hole, &heap->ordered_array);
    } else {
        // Remove the hole from the ordered array if it's completely used
        remove_ordered_array(hole_index, &heap->ordered_array);
    }

    // Mark the block as allocated
    hole_header->is_hole = 0;

    // Return the pointer to the usable memory (after the header)
    return (void *)((uint64_t)hole_header + sizeof(header_t) + offset);
}




void kfree(void *p, heap_t *heap) {
    if (p == NULL) {
        return; // Null pointer, nothing to free
    }

    // Retrieve the header of the block
    header_t *header = (header_t *)((uint64_t)p - sizeof(header_t));

    print("header->magic: ");
    print_hex(header->magic);
    print("\n");

    print("HEAP_MAGIC: ");
    print_hex(HEAP_MAGIC);
    print("\n");

    assert(header->magic == HEAP_MAGIC); // Ensure the block is valid



    // Mark the block as a hole
    header->is_hole = 1;

    // Retrieve the footer of the block
    footer_t *footer = (footer_t *)((uint64_t)header + header->size - sizeof(footer_t));
    assert(footer->magic == HEAP_MAGIC); // Ensure the footer is valid

    // Attempt to merge with the next block if it's a hole
    header_t *next_header = (header_t *)((uint64_t)footer + sizeof(footer_t));
    if ((uint64_t)next_header < heap->end_address && next_header->is_hole) {
        // Merge with the next block
        footer_t *next_footer = (footer_t *)((uint64_t)next_header + next_header->size - sizeof(footer_t));
        assert(next_footer->magic == HEAP_MAGIC);

        // Remove the next hole from the ordered array
        uint64_t iterator = 0;
        while (iterator < heap->ordered_array.size) {
            if (lookup_ordered_array(iterator, &heap->ordered_array) == next_header) {
                remove_ordered_array(iterator, &heap->ordered_array);
                break;
            }
            iterator++;
        }

        // Expand the current hole to include the next block
        header->size += next_header->size;
        footer = next_footer; // Update the footer
        footer->header = header;
    }

    // Attempt to merge with the previous block if it's a hole
    footer_t *prev_footer = (footer_t *)((uint64_t)header - sizeof(footer_t));
    if ((uint64_t)prev_footer >= heap->start_address && prev_footer->magic == HEAP_MAGIC) {
        header_t *prev_header = prev_footer->header;
        if (prev_header->is_hole) {
            // Remove the current hole from the ordered array
            uint64_t iterator = 0;
            while (iterator < heap->ordered_array.size) {
                if (lookup_ordered_array(iterator, &heap->ordered_array) == header) {
                    remove_ordered_array(iterator, &heap->ordered_array);
                    break;
                }
                iterator++;
            }

            // Expand the previous hole to include the current block
            prev_header->size += header->size;
            footer->header = prev_header; // Update the footer
            header = prev_header;        // Update the current header
        }
    }

    // Insert the merged (or unmerged) hole into the ordered array
    insert_ordered_array((void *)header, &heap->ordered_array);
}

void free(void *p) {
    kfree(p, kheap);
}

void heap_test(){
    // void *alloc(uint64_t size, uint8_t page_align, heap_t *heap)
    void *a = alloc(8, 0, kheap);
    void *b = alloc(8, 0, kheap);
    void *c = alloc(8, 0, kheap);

    print("a: ");
    print_dec((uint64_t)a);
    print(", b: ");
    print_dec((uint64_t)b);
    print(", c: ");
    print_dec((uint64_t)c);
    print("\n");

    free(&c);
    free(&b);

    void *d = alloc(12, 1, kheap);

    print(", d: ");
    print_hex((uint64_t)d);
    print("\n");
}