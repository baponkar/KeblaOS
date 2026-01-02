ENTRY(_start)

SECTIONS {
    . = 0x401000;       /* beginning of the region */

    .text : {
        *(.text.user)
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss COMMON)
    }

    /DISCARD/ : { *(.eh_frame) *(.comment) }
}
