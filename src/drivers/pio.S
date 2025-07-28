#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified


.equ INSTR_MEM0_OFFSET, 0x48

/**
 * Load program to pio state machine
 *  r0 - program address begin
 *  r1 - program size
 *  r2 - pio block
 */
.thumb_func
.global load_pio_prog
.align 4
load_pio_prog:
        movs    r3, #1
        lsls    r3, #21
        muls    r3, r3, r2
        ldr     r2, PIO0_BASE
        add     r2, r2, r3

#ifdef ARCH_RP2040
        ldr     r3, PIO1_BASE
#elifdef ARCH_RP2350
        ldr     r3, PIO2_BASE
#endif
        cmp     r2, r3
        itt     gt
        movsgt  r0, #1
        bgt     .exit

        .copy_loop:
                cmp     r1, #0
                itt     eq
                movseq  r0, #0
                beq     .exit

                str     r0, [r2]
                adds    r0, r0, #4
                adds    r2, r2, #4
                subs    r1, r1, #1

                b       .copy_loop

        .exit:
                bx lr


PIO0_BASE:      .word 0x50200000
PIO1_BASE:      .word 0x50300000
#ifdef ARCH_RP2350
PIO2_BASE:      .word 0x50400000
#endif
