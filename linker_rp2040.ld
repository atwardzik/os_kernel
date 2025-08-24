MEMORY
{
        code (rx)               : ORIGIN = 0x20000000, LENGTH = 65536
        user_space (rwx)        : ORIGIN = 0x20010000, LENGTH = 28416
        stack (rw)              : ORIGIN = 0x20016f00, LENGTH = 8K
        screen_buffer (rw)      : ORIGIN = 0x20018f00, LENGTH = 6400
        vidram (w)              : ORIGIN = 0x2001A800, LENGTH = 153600
        data (rw)               : ORIGIN = 0x20040000, LENGTH = 4K
        bss (rw)                : ORIGIN = 0x20041000, LENGTH = 4K
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

        __stack_start           = ORIGIN(stack);
        __stack_end             = ORIGIN(stack) + LENGTH(stack) - 1;

        __screen_buffer_start   = ORIGIN(screen_buffer);
        __screen_buffer_length  = LENGTH(screen_buffer);

        __vidram_start          = ORIGIN(vidram);
        __vidram_length         = LENGTH(vidram);
}
