#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
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

#ifdef ARCH_RP2040
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
#endif

/**
 * Signed integer modulo:
 * r0 - divident
 * r1 - divisor
 */
.thumb_func
.global hw_mod
.align 4
hw_mod:
        push {lr}

#ifdef ARCH_RP2040
        bl   hw_divider_divmod_s32
        mov  r0, r1
#elifdef ARCH_RP2350
        sdiv r3, r0, r1
        muls r3, r3, r1
        subs r0, r0, r3
#endif

        pop  {pc}

/**
 * Signed integer div:
 * r0 - divident
 * r1 - divisor
 */
.thumb_func
.global hw_div
.align 4
hw_div:
        push {lr}

#ifdef ARCH_RP2040
        bl   hw_divider_divmod_s32
#elifdef ARCH_RP2350
        sdiv r0, r0, r1
#endif

        pop  {pc}



.align 4
SIO_BASE:           .word 0xd0000000
