
ENTRY(start)  /* Kernel entry point */

SECTIONS
{
    _start = .;
    /* Start the kernel at 1MB, which is typical for protected mode */
    . = 1M;

    /* This is the code section */
    .text : {
        code = .;         /* Define the start of the code section */
        *(.multiboot)     /* Multiboot header */
        *(.text*)         /* All .text sections from input files */
    }

    /* This is the data section */
    .data : {
        *(.data*)
    }

    /* This is the bss section */
    .bss : {
        bss = .;          /* Define the start of the bss section */
        *(COMMON)         /* Uninitialized data (BSS) */
        *(.bss*)
        end = .;          /* Define the end of the kernel */
    }

    /* Provide memory alignment and reserve extra space if necessary */
    _end = .;
    . = ALIGN(4K);
}
