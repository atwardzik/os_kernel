.cpu cortex-m0
.thumb
.syntax unified

.equ eol, 0x00
.equ endl, 0x0A
.equ carriage_return, 0x0D

/**
 * Writes single character to output stream
 */
.thumb_func
.global putc
.align 4
putc:
        push {lr}
        bl   uart_Tx

        cmp  r0, endl
        bne  .not_endl
        movs r0, carriage_return
        bl   uart_Tx

        .not_endl:

        pop  {pc}


/**
 * Writes str to output stream
 * r0 - string pointer
 */
.thumb_func
.global puts
.align 4
puts:
        push {r4-r7, lr}
        mov  r4, r0                     @ save pointer

        movs r5, #0                     @ loop counter
        .put_loop:
                ldrb r0, [r4, r5]
                cmp  r0, eol
                beq  .end_put_loop
                bl   putc
                adds r5, r5, #1
                b    .put_loop
        .end_put_loop:

        pop {r4-r7, pc}


/**
 * Reads single character from input stream
 */
.thumb_func
.global getc
.align 4
getc:
        push {lr}
        bl   uart_Rx
        @     bl   keyboard_receive_char
        pop  {pc}


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
        push {r4-r7, lr}
        mov  r4, r0                     @ save pointer
        mov  r5, r1                     @ save max size
        subs r5, r5, #1                 @ max_index = max_size - 1

        movs r6, #0                     @ loop counter
        .get_loop:
                bl   getc
                cmp  r0, carriage_return
                beq  .end_get_loop
                cmp  r6, r5
                blt  .save
                b    .get_loop

        .save:
                strb r0, [r4, r6]
                adds r6, r6, #1
                b    .get_loop

        .end_get_loop:
                movs r0, eol
                strb r0, [r4, r6]

        pop {r4-r7, pc}
