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

#if 0
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

#endif
/**
 * Reads str from input stream until newline or MAX_SIZE
 * Always writes the terminating null character.
 * The newline character, if found, is discarded and does not count toward the number of characters written to the buffer.
 * r0 - buffer pointer
 * r1 - MAX_SIZE
 */
.thumb_func
.global my_gets
.align 4
my_gets:
        push    {r4-r7, lr}
        subs    sp, sp, #8
        mov     r4, r0                          @ save pointer
        mov     r5, r1                          @ save max size
        subs    r5, r5, #1                      @ max_index = max_size - 1

        movs    r6, #0                          @ loop counter
        movs    r7, #0                          @ loop counter MAX
        .get_loop:
                @ bl      getc
                @ bl      my_getc
                bl      keyboard_receive_char

                ldr     r1, =0x1b5b43
                cmp     r0, r1
                beq     .arrow_right
                ldr     r1, =0x1b5b44
                cmp     r0, r1
                beq     .arrow_left

                str     r0, [sp]
                bl      write_byte
                ldr     r0, [sp]

                cmp     r0, endl                @ carriage_return OR endline
                beq     .end_get_loop
                cmp     r6, r5
                blt     .save
                b       .get_loop

        .save:
                cmp     r0, backspace
                beq     .save_backspace
                strb    r0, [r4, r6]
                adds    r6, r6, #1
                adds    r7, r7, #1
                b       .get_loop

        .save_in_the_middle:
                @ insert instead of replacing characters, beware of backspace...
        .save_middle_backspace:

        .save_backspace:
                cmp     r6, #0
                beq     .get_loop
                subs    r6, r6, #1
                subs    r7, r7, #1
                b       .get_loop

        .arrow_left:
                cmp     r6, #0
                beq     .get_loop

                bl      write_byte
                subs    r6, r6, #1
                b       .get_loop

        .arrow_right:
                cmp     r6, r7
                bge     .get_loop

                bl      write_byte
                adds    r6, r6, #1
                b       .get_loop

        .end_get_loop:
                movs    r0, eol
                strb    r0, [r4, r7]

        adds    sp, sp, #8
        movs    r7, r4
        pop     {r4-r7, pc}
