MEMORY
{
    code (rx) : ORIGIN = 0x20000000, LENGTH = 65536
    user_space (rwx) : ORIGIN = 0x20010000, LENGTH = 143360
    stack (rw) : ORIGIN = 0x20033000, LENGTH = 8K
    vidram (w) : ORIGIN = 0x20035000, LENGTH = 307200
    data (rw) : ORIGIN = 0x20080000, LENGTH = 4K
    bss (rw) : ORIGIN = 0x20081000, LENGTH = 4K
}

SECTIONS
{
    .text : {
        __text_start = .;
        *(.reset*)
        *(.text*)
        __text_end = .;
    } > code

    .vectors : {
        . = ALIGN(256);
        __vectors_start = .; 
        *(.vectors*)
    } > code

    .vidram : {
            *(.vidram*)
    } > vidram

    .data : {
       __data_start = .;
       *(.data)
       *(.data*)
       __data_end = .;
    } > data

    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss*)
        *(COMMON*)
        __bss_end = .;
    } > bss
}
