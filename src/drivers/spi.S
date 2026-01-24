#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified

.equ SSPCR0, 0x00
.equ SSPCR1, 0x04
.equ SSPDR, 0x08
.equ SSPSR, 0x0c
.equ SSPCPSR, 0x10
.equ SSPDMACR, 0x24


/**
 * Setup the SPI_TX. Final frequency according to datasheet: cf. 12.3.4.6.1. Bit rate generation pp. 1051
 *   r0 - spi hw block address
 *   r1 - prescaller in 2-254 (LSB=0!!!)
 *   r2 - post-divider  0-255
 *   r3 - SPI operating mode (0-3)
 */
.thumb_func
.global spi_init
.align 4
spi_init:
        push    {r4-r7, lr}
        mov     r4, r0
        mov     r5, r1
        mov     r6, r2
        mov     r7, r3

        bl      spi_set_baudrate

        mov     r0, r4
        bl      spi_disable

        adds    r0, r4, SSPCR0
        movs    r1, #7                  @ 8 bit data, CPOL=0, CPHA=0, Motorola; TODO: CPOL, CPHA depend on r3
        bl      atomic_set

        adds    r0, r4, SSPDMACR
        movs    r1, #3                  @ according to SDK spi_init - always enable
        bl      atomic_set

        mov     r0, r4
        bl      spi_enable

        pop     {r4-r7, pc}

/**
 * Setup the SPI_TX. Final frequency according to datasheet: cf. 12.3.4.6.1. Bit rate generation pp. 1051
 *   r0 - spi hw block address
 *   r1 - prescaller in 2-254 (LSB=0!!!)
 *   r2 - post-divider  0-255
 */
.thumb_func
.global spi_set_baudrate
.align 4
spi_set_baudrate:
        push    {r4-r6, lr}
        mov     r4, r0
        mov     r5, r1
        mov     r6, r2
        bl      spi_disable

        @ 1) set prescaller
        adds    r0, r4, SSPCPSR
        mov     r1, r5
        bl      atomic_set

        @ 2) set post-divider
        adds    r0, r4, SSPCR0
        lsls    r1, r6, #8              @ SCR = post-divider
        bl      atomic_set

        mov     r0, r4
        bl      spi_enable

        pop     {r4-r6, pc}


/**
 * Enables spi hw block
 *  - r0 - address of a block
 */
.thumb_func
.global spi_enable
.align 4
spi_enable:
        push    {lr}

        adds    r0, r0, SSPCR1
        movs    r1, #2                  @ SSE_ENABLE BIT
        bl      atomic_set

        pop     {pc}

/**
 * Disables spi hw block
 *  - r0 - address of a block
 */
.thumb_func
.global spi_disable
.align 4
spi_disable:
        push    {lr}

        adds    r0, r0, SSPCR1
        movs    r1, #2                  @ SSE_ENABLE BIT
        bl      atomic_clr

        pop     {pc}

/**
 * Determines spi block pointed by r0. Doesn't affect other registers
 */
.thumb_func
.global spi_determine_block
.align 4
spi_determine_block:
        cmp     r0, #0
        ite     eq
        ldreq   r0, SPI0_BASE
        ldrne   r0, SPI1_BASE

        bx      lr


/**
 * Transfers data through selected SPI block
 *  - r0 - spi block (0, 1)
 *  - r1 - data to be send
 */
.thumb_func
.global spi_tx
.align 4
spi_tx:
        push    {lr}

        bl      spi_determine_block
        str     r1, [r0, SSPDR]

        pop     {pc}


/**
 * Transfers data through selected SPI block
 *  - r0 - spi block (0, 1)
 *
 * Returns:
 *  Received value in r0
 */
.thumb_func
.global spi_rx
.align 4
spi_rx:
        push    {lr}
        bl      spi_determine_block

        mov     r1, 0xff
        str     r1, [r0, SSPDR]

        ldr     r0, [r0, SSPDR]

        pop     {pc}


.align 4
#ifdef ARCH_RP2040
SPI0_BASE:      .word 0x4003c000
SPI1_BASE:      .word 0x40040000
#elifdef ARCH_RP2350
SPI0_BASE:      .word 0x40080000
SPI1_BASE:      .word 0x40088000
#endif

