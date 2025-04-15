#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Add TSS entry structure
struct tss{ // 104 bytes is the minimum size of a TSS
    uint32_t reserved0;
    uint64_t rsp0;  // Ring 0 Stack Pointer
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;  // Interrupt Stack Entries
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((aligned(16)));
typedef struct tss tss_t;

void tss_init();

void print_tss(tss_t *tss);

