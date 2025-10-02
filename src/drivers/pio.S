#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified

.equ PIO_CTRL, 0x00
.equ PIO_FDEBUG, 0x08
.equ PIO_TXF0, 0x10
.equ INSTR_MEM0_OFFSET, 0x48
.equ INSTR_MAX_OFFSET, 0xc4

.equ SM0_CLKDIV, 0xc8
.equ SM0_EXECCTRL, 0xcc
.equ SM0_SHIFTCTRL, 0xd0
.equ SM0_ADDR, 0xd4
.equ SM0_INSTR, 0xd8
.equ SM0_PINCTRL, 0xdc
.equ NEXT_SM_OFFSET, 24

.equ SM0_EXECCTRL_OUT_STICKY_BIT, 17


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
        mov     r4, r2

        movs    r2, SM0_CLKDIV
        bl      determine_pio_sm_register
        cmp     r0, #0
        beq     .exit_clk_failure

        str     r4, [r0]
        movs    r0, #0

        .exit_clk:
                pop     {r4, pc}
        .exit_clk_failure:
                movs    r0, #1
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
        mov     r4, r2

        movs    r2, SM0_PINCTRL
        bl      determine_pio_sm_register
        cmp     r0, #0
        beq     .exit_pinctrl_failure

        str     r4, [r0]
        movs    r0, #0

        .exit_pinctrl:
                pop     {r4, pc}
        .exit_pinctrl_failure:
                movs    r0, #1
                pop     {r4, pc}

/**
 * Set the same pin direction for multiple consecutive pins for the PIO instance
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - pin start
 *  r3 - pin count
 */
.thumb_func
.global set_consecutive_pindirs_out
.align 4
set_consecutive_pindirs_out:
        push    {r4-r7, lr}
        subs    sp, sp, #24
        mov     r4, r0
        mov     r5, r1
        mov     r6, r2
        mov     r7, r3

        bl      determine_pio_block_address
        cmp     r0, #0
        beq     .exit_consecutive_pindirs
        str     r0, [sp]                        @ offset 0, store PIO block base
        ldr     r2, =0x168                      @ gpiobase offset
        ldr     r2, [r0, r2]
#ifdef ARCH_RP2350
        subs    r6, r6, r2                      @ according to SDK
#endif
        @ Save pinctrl
        mov     r0, r4
        mov     r1, r5
        movs    r2, SM0_PINCTRL
        bl      determine_pio_sm_register
        cmp     r0, #0                          @ we can check it once, as when it fails, all next will fail
                                                @ if it succeeds, all next will succeed as well.
        beq     .exit_consecutive_pindirs
        str     r0, [sp, #4]                    @ offset 4, store PINCTRL address
        ldr     r0, [r0]
        str     r0, [sp, #8]                    @ offset 8, store old PINCTRL

        @ Save execctrl
        mov     r0, r4
        mov     r1, r5
        movs    r2, SM0_EXECCTRL
        str     r0, [sp, #12]                   @ offset 12, store EXECCTRL address
        ldr     r0, [r0]
        str     r0, [sp, #16]                   @ offset 16, store old EXECCTRL

        @ Clear sticky bit
        ldr     r0, [sp]                        @ load base address
        movs    r1, #1
        lsls    r1, SM0_EXECCTRL_OUT_STICKY_BIT
        bl      atomic_clr

        @ Set outputs
        mov     r0, r4
        mov     r1, r5
        movs    r2, SM0_INSTR
        bl      determine_pio_sm_register

        .set_middle_pindirs:
                cmp     r7, #5
                ble     .set_last_pindirs
                movs    r1, #5                  @ five pins affected
                lsls    r1, #26                 @ SET_COUNT_BITS
                lsls    r2, r6, #5              @ pin_start << SET_BASE_BITS
                orrs    r2, r1, r2              @ control bits
                ldr     r1, [sp, #4]            @ load PINCTRL address
                str     r2, [r1]

                ldr     r1, instr_set_pindirs_out
                str     r1, [r0]

                subs    r7, r7, #5              @ pin_count -= 5
                adds    r6, r6, #5              @ pin_start += 5
                movs    r2, #0x1f
                ands    r6, r6, r2              @ pin_start &= 0x1f

                b       .set_middle_pindirs

        @ while (count > 5) {
        @         pio->sm[sm].pinctrl = (5u << PIO_SM0_PINCTRL_SET_COUNT_LSB) | (pin << PIO_SM0_PINCTRL_SET_BASE_LSB);
        @         pio_sm_exec(pio, sm, pio_encode_set(pio_pindirs, pindir_val));
        @         count -= 5;
        @         pin = (pin + 5) & 0x1f;
        @ }
        @ pio->sm[sm].pinctrl = (count << PIO_SM0_PINCTRL_SET_COUNT_LSB) | (pin << PIO_SM0_PINCTRL_SET_BASE_LSB);
        @ pio_sm_exec(pio, sm, pio_encode_set(pio_pindirs, pindir_val));

        .set_last_pindirs:
                lsls    r1, r7, #26             @ pin_count << SET_COUNT_BITS
                lsls    r2, r6, #5              @ pin_start << SET_BASE_BITS
                orrs    r2, r1, r2              @ control bits
                ldr     r1, [sp, #4]            @ load PINCTRL address
                str     r2, [r1]

                ldr     r1, instr_set_pindirs_out
                str     r1, [r0]                        @ WILL IT WORK IF THE SM IS HALTED?!
                                                        @ According to the SDK, PIO: CTRL register bit 0:3
                                                        @ When disabled, a state machine will cease executing
                                                        @ instructions, except those written directly to
                                                        @ SMx_INSTR by the system.
        @ Restore PINCTRL
        ldr     r0, [sp, #4]
        ldr     r1, [sp, #8]
        str     r1, [r0]

        @ Restore EXECCTRL
        ldr     r0, [sp, #12]
        ldr     r1, [sp, #16]
        str     r1, [r0]


        .exit_consecutive_pindirs:
                adds    sp, sp, #24
                pop     {r4-r7, pc}


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
        push    {r4, r5, lr}
        mov     r4, r2

        movs    r2, SM0_EXECCTRL
        bl      determine_pio_sm_register
        cmp     r0, #0
        beq     .exit_execctrl_failure
        mov     r5, r0

        ldr     r1, =0x1ff80                    @ clear wrap bits
        bl      atomic_clr

        mov     r0, r5
        mov     r1, r4
        bl      atomic_set
        movs    r0, #0

        .exit_execctrl:
                pop     {r4, r5, pc}

        .exit_execctrl_failure:
                movs    r0, #1
                pop     {r4, r5, pc}

/**
 * Setup PIO shiftctrl for specified state machine pp. PIO0_SHIFTCTRL
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - control bits
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
.thumb_func
.global setup_pio_shiftctrl
.align 4
setup_pio_shiftctrl:
        push    {r4, lr}
        mov     r4, r2

        movs    r2, SM0_SHIFTCTRL
        bl      determine_pio_sm_register
        cmp     r0, #0
        beq     .exit_shiftctrl_failure
        mov     r5, r0

        str     r4, [r0]
        movs    r0, #0

        .exit_shiftctrl:
                pop     {r4, pc}

        .exit_shiftctrl_failure:
                movs    r0, #1
                pop     {r4, pc}

/**
 * Clear current fifo for specified state machine and clear PIO FDEBUG register
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
.thumb_func
.global clear_pio_sm_fifo
.align 4
clear_pio_sm_fifo:
        push    {r4-r7, lr}
        mov     r4, r0
        mov     r5, r1

        movs    r2, SM0_SHIFTCTRL
        bl      determine_pio_sm_register
        cmp     r0, #0
        beq     .exit_clear_fifo_failure
        mov     r6, r0
        movs    r1, #1
        lsls    r1, r1, #31
        bl      atomic_xor
        mov     r0, r6
        bl      atomic_xor


        mov     r0, r4
        bl      determine_pio_block_address
        cmp     r0, #0
        beq     .exit_clear_fifo_failure
        adds    r0, r0, PIO_FDEBUG

        ldr     r1, =0x1010101                  @ mask for clearing fdebug
        lsls    r1, r1, r5                      @ clear fdebug for specified sm

        str     r1, [r0]

        movs    r0, #0
        pop     {r4-r7, pc}

        .exit_clear_fifo_failure:
                movs    r0, #1
                pop     {r4-r7, pc}


/**
 * Clear internal state and jump to code
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - address begin, initial pc (max 0x1f)
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
.thumb_func
.global clear_internal_and_jump
.align 4
clear_internal_and_jump:
        push    {r4-r7, lr}
        mov     r4, r0
        mov     r5, r1
        mov     r6, r2

        bl      determine_pio_block_address
        cmp     r0, #0
        beq     .exit_clear_jump_failure
        adds    r0, r0, PIO_CTRL
        mov     r7, r0

        movs    r1, #1
        movs    r2, #8                          @ SM_RESTART_BIT
        adds    r2, r2, r5
        lsls    r1, r1, r2
        bl      atomic_set

        mov     r0, r7
        movs    r1, #1
        movs    r2, #0x80                       @ CLKDIV_RESTART_BIT
        adds    r2, r2, r5
        lsls    r1, r1, r2
        bl      atomic_set

        @ TBH pure address of instruction is also encoded version of jump to this address
        mov     r0, r4
        mov     r1, r5
        movs    r2, SM0_INSTR
        bl      determine_pio_sm_register
        cmp     r0, #0
        beq     .exit_clear_jump_failure
        str     r6, [r0]                        @ jump

        movs    r0, #0
        pop     {r4-r7, pc}

        .exit_clear_jump_failure:
                movs    r0, #1
                pop     {r4-r7, pc}


/**
 * Put a word to the fifo for specified sm
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - word
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
.thumb_func
.global sm_put
.align 4
sm_put:
        push    {lr}

        bl      determine_pio_block_address
        cmp     r0, #0
        beq     .sm_put_failure
        movs    r3, #4
        muls    r3, r3, r1
        adds    r0, r0, PIO_TXF0
        adds    r0, r0, r3

        str     r2, [r0]

        movs    r0, #0
        pop     {pc}

        .sm_put_failure:
                movs    r0, #1
                pop     {pc}

/**
 * Set State Machine Enabled
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - enabled?
 *
 * Returns:
 *  0 - success
 *  1 - failure, no such PIO or SM exists
 */
.thumb_func
.global set_sm_enabled
.align 4
set_sm_enabled:
        push    {r4, lr}
        mov     r4, r1

        bl      determine_pio_block_address
        cmp     r0, #0
        beq     .exit_sm_enabled_failure
        cmp     r1, #3
        bgt     .exit_sm_enabled_failure



        movs    r1, #1
        lsls    r1, r4
        cmp     r2, #0
        beq     .sm_disable
        bl      atomic_set
        b       .exit_sm_enabled_success
        .sm_disable:
                bl      atomic_clr


        .exit_sm_enabled_success:
                movs    r0, #0
                pop     {r4, pc}

        .exit_sm_enabled_failure:
                movs    r0, #1
                pop     {r4, pc}


/**
 * Sets multiple state machines enabled in sync.
 *  r0 - PIO BLOCK
 *  r1 - state machines bits
 *  r2 - enabled?
 */
.thumb_func
.global set_multiple_sm_enabled
.align 4
set_multiple_sm_enabled:
        push    {lr}

        bl      determine_pio_block_address
        cmp     r0, #0
        beq     .exit_multiple_sm_enabled_failure
        cmp     r1, #15
        bgt     .exit_multiple_sm_enabled_failure

        mov     r2, r1
        lsls    r1, r1, #8                              @ reset state machines clocks
        adds    r1, r1, r2                              @ enable bits

        cmp     r2, #0
        beq     .multiple_sm_disable
        bl      atomic_set
        b       .exit_multiple_sm_enabled_success
        .multiple_sm_disable:
                bl      atomic_clr

        .exit_multiple_sm_enabled_success:
                movs    r0, #0
                pop     {pc}

        .exit_multiple_sm_enabled_failure:
                movs    r0, #1
                pop     {pc}

/**
 * Determine specified PIO register
 *  r0 - PIO BLOCK
 *  r1 - state machine
 *  r2 - register
 *
 * Returns:
 *  NULL(0) - failure, no such PIO or SM exists
 *  address - success
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
        adds    r0, r0, r1

        b       .exit_reg_success

        .exit_reg_failure:
                movs    r0, #0
                pop     {pc}
        .exit_reg_success:

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

instr_set_pindirs_out:          .word 0xe09f    @ set PINDIRS, 0x1f

.section .data
.align 4
current_pio0_instr_offset:      .byte 0
current_pio1_instr_offset:      .byte 0

current_pio0_sm:                .byte 0
current_pio1_sm:                .byte 0

#ifdef ARCH_RP2350
current_pio2_instr_offset:      .byte 0
current_pio2_sm:                .byte 0
#endif
