#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified

.equ eol, 0x00
.equ endl, 0x0A
.equ carriage_return, 0x0D
.equ backspace, 0x08
.equ empty_space, 0x20

.equ TIMER_ALARM0_OFFSET, 0x10
.equ TIMER_TIMERAWL_OFFSET, 0x28
#ifdef ARCH_RP2040
.equ TIMER_INTE, 0x38
.equ TIMER_INTR, 0x34
#elifdef ARCH_RP2350
.equ TIMER_INTR, 0x3c
.equ TIMER_INTE, 0x40
#endif

/**
 * Writes str to output stream
 * r0 - string pointer
 */
.thumb_func
.global puts
.align 4
puts:
        push    {r4-r7, lr}
        mov     r4, r0                     @ save pointer

        movs    r5, #0                     @ loop counter
        .put_loop:
                ldrb    r0, [r4, r5]
                cmp     r0, eol
                beq     .end_put_loop
                bl      putc
                adds    r5, r5, #1
                b       .put_loop
        .end_put_loop:

        pop     {r4-r7, pc}


/**
 * Reads single character from input stream
 */
@ .thumb_func
@ .global getc
@ .align 4
@ getc:
@         push    {lr}
@         @ bl      uart_Rx
@         bl      keyboard_receive_char
@         pop     {pc}


/**
 * Reads str from input stream until newline or MAX_SIZE
 * Always writes the terminating null character.
 * The newline character, if found, is discarded and does not count toward the number of characters written to the buffer.
 * r0 - buffer pointer
 * r1 - MAX_SIZE
 */
.thumb_func
.global gets
.align 4
gets:
        push    {r4-r7, lr}
        mov     r4, r0                     @ save pointer
        mov     r5, r1                     @ save max size
        subs    r5, r5, #1                 @ max_index = max_size - 1

        movs    r6, #0                     @ loop counter
        .get_loop:
                bl      getc

                mov     r7, r0
                bl      putc
                mov     r0, r7

                cmp     r0, endl           @ carriage_return OR endline
                beq     .end_get_loop
                cmp     r6, r5
                blt     .save
                b       .get_loop

        .save:
                cmp     r0, backspace
                beq     .save_backspace
                strb    r0, [r4, r6]
                adds    r6, r6, #1
                b       .get_loop

        .save_backspace:
                cmp     r6, #0
                beq     .get_loop
                subs    r6, r6, #1
                b       .get_loop

        .end_get_loop:
                movs    r0, eol
                strb    r0, [r4, r6]

        pop     {r4-r7, pc}


.thumb_func
.global cursor_blinker_handler
.align 4
cursor_blinker_handler:
        cpsid   i
        push    {r4, lr}

        ldr     r4, =cursor_blink_time
        ldr     r4, [r4]
        @ 1) xor cursor pointer
        bl      xor_cursor

        @ 2) reset
        ldr     r3, TIMER_BASE
        adds    r3, r3, TIMER_INTR
        movs    r2, #1
        str     r2, [r3]                                @ reset interrupt for alarm0

        ldr     r3, TIMER_BASE
        adds    r3, r3, TIMER_INTE
        movs    r2, #1                                  @ enable timer interrupt for alarm0 i.e. (1 << 0)
        str     r2, [r3]

        ldr     r3, TIMER_BASE
        ldr     r1, [r3, TIMER_TIMERAWL_OFFSET]         @ get TIMERAWL
        add     r1, r1, r4                              @ TIMERAWL + desired alarm
        str     r1, [r3, TIMER_ALARM0_OFFSET]

        cpsie   i
        pop     {r4, pc}

/**
 * Starts the cursor blinking.
 *   r0 - blinking delay
 */
.thumb_func
.global set_cursor_blink
.align 4
set_cursor_blink:
        push    {r4, lr}
        mov     r4, r0
        ldr     r1, =cursor_blink_time
        str     r0, [r1]

        movs    r0, #0
        ldr     r1, =cursor_blinker_handler
        bl      set_isr

        ldr     r0, TIMER_BASE
        movs    r1, #1                                  @ enable timer interrupt for alarm0 i.e. (1 << 0)
        str     r1, [r0, TIMER_INTE]

        ldr     r1, [r0, TIMER_TIMERAWL_OFFSET]         @ get TIMERAWL
        add     r1, r1, r4                              @ TIMERAWL + desired alarm
        str     r1, [r0, TIMER_ALARM0_OFFSET]

        pop     {r4, pc}

.thumb_func
.global set_cursor_off
.align 4
set_cursor_off:
        ldr     r0, TIMER_BASE
        movs    r1, #0                                  @ disable timer interrupt for alarm0 i.e. (0 << 0)
        str     r1, [r0, TIMER_INTE]

        bx      lr


#ifdef ARCH_RP2040
.align 4
TIMER_BASE:             .word 0x40054000
#elifdef ARCH_RP2350
.align 4
TIMER_BASE:             .word 0x400b0000
#endif

.section .data
cursor_blink_time:      .word 0
