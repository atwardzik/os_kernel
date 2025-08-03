#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified

.equ PIO_CTRL, 0x00
.equ INSTR_MEM0_OFFSET, 0x48
.equ INSTR_MAX_OFFSET, 0xc4

.equ SM0_CLKDIV, 0xc8
.equ SM0_EXECCTRL, 0xcc
.equ SM0_SHIFTCTRL, 0xd0
.equ SM0_ADDR, 0xd4
.equ SM0_INSTR, 0xd8
.equ SM0_PINCTRL, 0xdc
.equ NEXT_SM_OFFSET, 24


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
                strbeq  r4, [r3]
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



/**
 * Setup clock divisor for specified state machine
 *      Freq = clock freq / (CLKDIV_INT + CLKDIV_FRAC / 256)
 *      31:16 CLKDIV_INT
 *      15:8  CLKDIV_FRAC
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - clock divisor
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
.thumb_func
.global setup_pio_clkdiv
.align 4
setup_pio_clkdiv:
        push    {r4, lr}
        mov     r2, r4

        mov     r2, SM0_CLKDIV
        bl      determine_pio_sm_register
        cmp     r0, #1
        beq     .exit_clk

        str     r4, [r0]
        movs    r0, #0

        .exit_clk:
                pop     {r4, pc}


/**
 * Setup PIO pinctrl for specified state machine pp. PIO0_PINCTRL
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - control bits
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
.thumb_func
.global setup_pio_pinctrl
.align 4
setup_pio_pinctrl:
        push    {r4, lr}
        mov     r2, r4

        mov     r2, SM0_PINCTRL
        bl      determine_pio_sm_register
        cmp     r0, #1
        beq     .exit_pinctrl

        str     r4, [r0]
        movs    r0, #0

        .exit_pinctrl:
                pop     {r4, pc}

/**
 * Setup PIO execctrl for specified state machine pp. PIO0_EXECCTRL
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - control bits
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
.thumb_func
.global setup_pio_execctrl
.align 4
setup_pio_execctrl:
        push    {r4, lr}
        mov     r2, r4

        mov     r2, SM0_EXECCTRL
        bl      determine_pio_sm_register
        cmp     r0, #1
        beq     .exit_execctrl

        str     r4, [r0]
        movs    r0, #0

        .exit_execctrl:
                pop     {r4, pc}


/**
 * Set State Machine Running
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
 .thumb_func
.global set_sm_running
.align 4
set_sm_running:
        push    {r4, lr}
        movs    r4, r1

        cmp     r0, #0
        beq     .exit_sm_set_failure
        cmp     r1, #3
        bgt     .exit_sm_set_failure



        ldr     r1, =0x2000             @ atomic bitmask set
        adds    r0, r0, r1
        str     r4, [r0]
        movs    r0, #0
        pop     {r4, pc}

        .exit_sm_set_failure:
                movs    r0, #1
                pop     {r4, pc}


/**
 * Determine specified PIO register
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - register
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
.thumb_func
.global determine_pio_sm_register
.align 4
determine_pio_sm_register:
        push    {lr}
        bl      determine_pio_block_address
        cmp     r0, #0
        beq     .exit_reg_failure
        cmp     r1, #3
        bgt     .exit_reg_failure


        movs    r3, NEXT_SM_OFFSET
        muls    r1, r1, r3
        adds    r1, r1, r2

        b       .exit_reg_success

        .exit_reg_failure:
                movs    r0, #1
                pop     {pc}
        .exit_reg_success:
                movs    r0, #0
                pop     {pc}



/**
 * Determines address for PIO block
 *  r0 - PIO block number
 *
 * Doesn't affect other registers
 *
 * Returns:
 *  NULL (0) - failure
 *  PIO block address
 */
.thumb_func
.align 4
determine_pio_block_address:
        cmp     r0, #0
        beq     .block_case_0
        cmp     r0, #1
        beq     .block_case_1
#ifdef ARCH_RP2350
        cmp     r0, #2
        beq     .block_case_2
#endif

        .block_case_default:
                movs    r0, #0
                bx      lr
        .block_case_0:
                ldr     r0, PIO0_BASE
                bx      lr
        .block_case_1:
                ldr     r0, PIO1_BASE
                bx      lr
#ifdef ARCH_RP2350
        .block_case_2:
                ldr     r0, PIO2_BASE
                bx      lr
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
