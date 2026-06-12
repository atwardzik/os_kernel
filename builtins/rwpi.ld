PHDRS
{
    text  PT_LOAD FLAGS(5);   /* R+X */
    rw    PT_LOAD FLAGS(6);   /* R+W */
}

SECTIONS
{
    . = 0;

    .data : {
        __data_start = .;
        *(.data*)
        __data_end = .;
    } :rw

    .bss : {
        __bss_start = .;
        *(.bss* COMMON)
        __bss_end = .;
    } :rw

    .text : {
        *(.text*)
        *(.rodata*)
    } :text
}
