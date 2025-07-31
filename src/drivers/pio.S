#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified


.equ INSTR_MEM0_OFFSET, 0x48
.equ INSTR_MAX_OFFSET, 0xc4

/**
 * Load program to pio state machine
 *  r0 - program address begin
 *  r1 - program size
 *  r2 - pio block
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO block exists
 *  2 - failure, out of memory for current PIO block. Using loaded program is undefined behaviour.
 *      Current instruction offset is not modyfied - calling this function again with proper program
 *      size will store the program correctly.
 */
.thumb_func
.global load_pio_prog
.align 4
load_pio_prog:
        push    {r4-r7, lr}
        @ Determine address for instruction storage
        @       r2 - PIO offset for instruction storage
        @       r3 - next instruction offset
        cmp     r2, #0
        beq     .case_0
        cmp     r2, #1
        beq     .case_1
#ifdef ARCH_RP2350
        cmp     r2, #2
        beq     .case_2
#endif

        .case_default:
                movs    r0, #1
                pop     {r4-r7, pc}
        .case_0:
                ldr     r2, PIO0_BASE
                adds    r2, INSTR_MEM0_OFFSET
                ldr     r3, =current_pio0_instr_offset

                b       .end_case
        .case_1:
                ldr     r2, PIO1_BASE
                adds    r2, INSTR_MEM0_OFFSET
                ldr     r3, =current_pio1_instr_offset

                b       .end_case
#ifdef ARCH_RP2350
        .case_2:
                ldr     r2, PIO2_BASE
                adds    r2, INSTR_MEM0_OFFSET
                ldr     r3, =current_pio2_instr_offset

                b       .end_case
#endif

        .end_case:
                ldrb    r4, [r3]        @ get current pioX instruction offset

@@@ It is left as is, because it was a test for IT instruction
@@@     and I don't want to delete this elegant assembly code.
@@@ Normally one would leave code backward-compatible (for rp2040)
#ifdef ARCH_RP2040
        .copy_loop:
                @ out of memory?
                cmp     r4, INSTR_MAX_OFFSET
                bgt     .exit_failure

                @ load finished?
                cmp     r1, #0
                beq     .exit_success

                @ copy
                ldr     r5, [r0]
                str     r5, [r2, r4]
                adds    r0, r0, #4
                adds    r4, r4, #4
                subs    r1, r1, #1

                b       .copy_loop

        .exit_failure:
                movs    r0, #2
                pop     {r3-r7, pc}

        .exit_success:
                @ Save next instruction offset
                strb    r4, [r3]

                movs    r0, #0
                pop     {r3-r7, pc}

#elifdef ARCH_RP2350
        .copy_loop:
                @ out of memory?
                cmp     r4, INSTR_MAX_OFFSET
                itt     eq
                movseq  r0, #2
                beq     .exit

                @ load finished?
                cmp     r1, #0
                ittt    eq
                strb    r4, [r3]
                movseq  r0, #0
                beq     .exit

                @ copy
                ldr     r5, [r0]
                str     r5, [r2, r4]
                adds    r0, r0, #4
                adds    r4, r4, #4
                subs    r1, r1, #1

                b       .copy_loop

        .exit:
                pop     {r4-r7, pc}
#endif


.align 4
PIO0_BASE:      .word 0x50200000
PIO1_BASE:      .word 0x50300000
#ifdef ARCH_RP2350
PIO2_BASE:      .word 0x50400000
#endif

.section data
.align 4
current_pio0_instr_offset:      .byte 0
current_pio1_instr_offset:      .byte 0

current_pio0_sm:                .byte 0
current_pio1_sm:                .byte 0

#ifdef ARCH_RP2350
current_pio2_instr_offset:      .byte 0
current_pio2_sm:                .byte 0
#endif
