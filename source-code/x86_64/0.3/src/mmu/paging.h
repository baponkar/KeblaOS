#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../driver/vga/vga.h"
#include "../stdlib/stdio.h"

#include "../util/util.h"



struct PML4{
    uint64_t p      :1;
    uint64_t rw     :1;
    uint64_t us     :1;
    uint64_t pwt    :1;
    uint64_t pcd    :1;
    uint64_t avl_1  :1;
    uint64_t rsvd_1 :3;
    uint64_t avl_2  :3;
    uint64_t tab_addr :28;
    uint64_t rsvd_2 :12;
    uint64_t avl_3  :7;
    uint64_t avl_4  :4;
    uint64_t xd     :1;
}__attribute__((packed));
typedef struct PML4 pml4_t;


struct PDPT{
    uint64_t p      :1;
    uint64_t rw     :1;
    uint64_t us     :1;
    uint64_t pwt    :1;
    uint64_t pcd    :1;
    uint64_t avl_1  :1;
    uint64_t rsvd_1 :3;
    uint64_t avl_2  :3;
    uint64_t tab_addr :28;
    uint64_t rsvd_2 :12;
    uint64_t avl_3  :7;
    uint64_t avl_4  :4;
    uint64_t xd     :1;
}__attribute__((packed));
typedef struct PDPT pdpt_t;


struct pd{
    uint64_t p      :1;
    uint64_t rw     :1;
    uint64_t us     :1;
    uint64_t pwt    :1;
    uint64_t pcd    :1;
    uint64_t avl_1  :1;
    uint64_t rsvd_1 :3;
    uint64_t avl_2  :3;
    uint64_t tab_addr :28;
    uint64_t rsvd_2 :12;
    uint64_t avl_3  :7;
    uint64_t avl_4  :4;
    uint64_t xd     :1;
}__attribute__((packed));
typedef struct pd pd_t;


struct pt{
    uint64_t p          :1;
    uint64_t rw         :1;
    uint64_t us         :1;
    uint64_t pwt        :1;
    uint64_t pcd        :1;
    uint64_t a          :1;
    uint64_t d          :1;
    uint64_t pat        :1;
    uint64_t g          :1;
    uint64_t avl_1      :3;
    uint64_t page_addr  :28;
    uint64_t rsvd       :12;
    uint64_t avl_2      :7;
    uint64_t avl_3      :4;
    uint64_t xd         :1;
}__attribute__((packed));
typedef struct pt pt_t;



void init_paging();
void page_fault_handler(registers_t *regs);
uint64_t get_phys_addr(uint64_t vir_addr, pml4_t* pml4_base);
void test_paging();

void test_paging();
