ENTRY(_start)
SECTIONS
{
    . = 0x80000000;
    .text : {
        *(.text*)
    }
    .rodata : {
        *(.rodata*)
    }
    .data : {
        *(.data*)
    }
    _edata = .;
    .bss : {
        *(.bss* .sbss*)
    }
    _end = .;
    PROVIDE(__global_pointer$ = .);
    PROVIDE (_stack = 0x84000000);
}
