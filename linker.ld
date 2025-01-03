MEMORY
{
    sram_code (rx) : ORIGIN = 0x20000000, LENGTH = 256k     /*four larger banks*/
    sram_data (rw) : ORIGIN = 0x20040000, LENGTH = 4k       /*smaller data bank*/
}

SECTIONS
{
    .text : {
        *(.reset*)
        *(.text*)
    } > sram_code

    .vectors : {
        . = ALIGN(256);
        *(.vectors*)
    } > sram_code

    .data : {
       __data_start = .;
       *(.data)
       *(.data.*)
       __data_end = .;
    } > sram_data
}