.cpu cortex-m0
.thumb

.syntax unified

.equ DIV_UDIVIDENT, 0x60
.equ DIV_UDIVISOR, 0x64
.equ DIV_SDIVIDENT, 0x68
.equ DIV_SDIVISOR, 0x6c
.equ DIV_QUOTIENT, 0x70
.equ DIV_REMAINDER, 0x74
.equ DIV_CSR, 0x78

//TODO: use pico bootrom for operating on floating point numbers


/**
 * Signed integer divider:
 * r0 - divident
 * r1 - divisor
 *
 * Returns:
 * r0 - quotient
 * r1 - remainder
 */
.thumb_func
.align 4
hw_divider_divmod_s32:
    ldr  r2, SIO_BASE
    str  r0, [r2, DIV_SDIVIDENT]
    str  r1, [r2, DIV_SDIVISOR]

    movs r1, #1                     @ bit 0 - calculation done
    .wait_for_calc:
        ldr  r0, [r2, DIV_CSR]
        tst  r0, r1
        beq  .wait_for_calc

    ldr  r0, [r2, DIV_QUOTIENT]
    ldr  r1, [r2, DIV_REMAINDER]

    bx   lr

/**
 * Signed integer modulo:
 * r0 - divident
 * r1 - divisor
 */
.thumb_func
.global mod
.align 4
mod:
    push {lr}

    bl   hw_divider_divmod_s32
    mov r0, r1

    pop  {pc}

/**
 * Signed integer div:
 * r0 - divident
 * r1 - divisor
 */
.thumb_func
.global div
.align 4
div:
    push {lr}

    bl   hw_divider_divmod_s32

    pop  {pc}



.align 4
SIO_BASE:           .word 0xd0000000
