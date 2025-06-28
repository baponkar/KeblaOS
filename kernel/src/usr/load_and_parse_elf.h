#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


// ELF headers
#define PT_LOAD 1

// ELF 64 Header
typedef struct {
    uint8_t  e_ident[16];   // ELF identification
    uint16_t e_type;        // Object file type
    uint16_t e_machine;     // Machine type
    uint32_t e_version;     // Object file version    
    uint64_t e_entry;       // Entry point address
    uint64_t e_phoff;       // Program header table file offset
    uint64_t e_shoff;       // Section header table file offset
    uint32_t e_flags;       // Processor-specific flags
    uint16_t e_ehsize;      // ELF header size
    uint16_t e_phentsize;   // Program header table entry size
    uint16_t e_phnum;       // Number of program header entries
    uint16_t e_shentsize;   // Section header table entry size
    uint16_t e_shnum;       // Number of section header entries
    uint16_t e_shstrndx;     // Section header string table index
} __attribute__((packed)) Elf64_Ehdr;

// ELF 64 Program Header
typedef struct {
    uint32_t p_type;    // Segment type
    uint32_t p_flags;   // Segment flags
    uint64_t p_offset;  // Segment file offset
    uint64_t p_vaddr;   // Segment virtual address
    uint64_t p_paddr;   // Segment physical address
    uint64_t p_filesz;  // Segment size in file
    uint64_t p_memsz;   // Segment size in memory
    uint64_t p_align;   // Segment alignment
} __attribute__((packed)) Elf64_Phdr;


void get_kernel_modules_info();
void print_kernel_modules_info();

void load_user_elf_and_jump();






