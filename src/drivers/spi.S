#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified


/**
 * Setup the SPI_TX
 *   r0 - pin
 */
.thumb_func
.global spi_Tx_init
.align 4
spi_Tx_init:
        bx      lr



.align 4
spi_Tx:
                        @       .wrap_target
        .word 0x80a0    @ 0:            pull block
        .word 0x6021    @
