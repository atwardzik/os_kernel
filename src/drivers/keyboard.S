#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified


#include "kernel/syscalls.h"
#include "kernel/resources_codes.h"

/**
 * +--------------+------------------+---------------------+
 * | Key          | (Set 2) Make Code| (Set 2) Break Code  |
 * +--------------+------------------+---------------------+
 * | "A"          | 1C               | F0,1C               |
 * | "5"          | 2E               | F0,2E               |
 * | "F10"        | 09               | F0,09               |
 * | Right Arrow  | E0,74            | E0,F0,74            |
 * | Right "Ctrl" | E0,14            | E0,F0,14            |
 * +--------------+------------------+---------------------+
 */

@ Please note that the pins are consecutive, e.g. when starting at pin 2, pins {2,3,4,5} will be treaded as input
.equ UPPER_NIBBLE_START, 2
.equ LOWER_NIBBLE_START, 6
.equ NIBBLE_MASK, 0x0F


.equ GPIO_IN, 0x04
#ifdef ARCH_RP2040
.equ IO_IRQ_BANK0, 13
#elifdef ARCH_RP2350
.equ IO_IRQ_BANK0, 21
#endif

.equ ARROW_UP, 0x41
.equ ARROW_DOWN, 0x42
.equ ARROW_RIGHT, 0x43
.equ ARROW_LEFT, 0x44

#ifdef KEYBOARD_PARALLEL_INTERFACE
/**
 * Initialize interrupts and data input for keyboard with PS-2 connector.
 * r0 - keyboard interrupt pin
 */
.thumb_func
.global init_keyboard
.align 4
init_keyboard:
        push    {r4, r5, lr}
        mov     r4, r0

        ldr     r1, =interrupt_pin
        str     r0, [r1]

        @ determine bit for edge high -> (pin % 8) * 4 + 3
        ldr     r0, =edge_high_bit
        movs    r2, #0x07
        ands    r1, r4, r2                         @ equivalent to (pin % 8)

        movs    r2, #0x04
        muls    r1, r1, r2
        adds    r1, r1, #0x03

        movs    r2, #1
        lsls    r2, r2, r1

        mov     r5, r2
        str     r2, [r0]

        .init_pin_interrupt:
                @ 1) GPIO INIT
                mov     r0, r4
                movs    r1, #5                         @ 5 - SIO
                bl      GPIO_function_select

                @ 2) CONFIGURATION
                mov     r0, r4
                movs    r1, #1                         @ PULL UP
                bl      init_pin_input_with_pull

                @ 3) GPIO_SET_IRQ_ENABLED
                @ 3.1) acknowledge_irq - "clear stale events which might cause immediate spurious handler entry"
                ldr     r1, IO_BANK0_BASE

                mov     r0, r4
                lsrs    r0, r0, #3                     @ pin / 8 = INTR_n
                movs    r2, #0x04
                muls    r0, r0, r2
                ldr     r3, INTR_OFFSET
                add     r0, r0, r3
                add     r0, r0, r1

                str     r5, [r0]                       @ clear by setting INTR

                @ 3.2) set irq enabled
                ldr     r0, IO_BANK0_BASE

                lsrs    r1, r4, #3                     @ pin / 8 = INTE_n
                movs    r2, #0x04
                muls    r1, r1, r2

                ldr     r2, PROC0_INTE0_OFFSET
                add     r1, r1, r2
                add     r0, r0, r1

                str     r5, [r0]                       @ set irq enabled - INTE

                @ 4) set interrupt subroutine
                movs    r0, IO_IRQ_BANK0
                ldr     r1, =keyboard_interrupt_handler
                bl      set_isr

        .init_keyboard_input_pins:
                .init_upper_word:
                movs    r0, UPPER_NIBBLE_START
                movs    r1, #4                         @ counter
                movs    r2, #1                         @ PULL UP
                bl      init_multiple_pin_inputs

                .init_lower_word:
                movs    r0, LOWER_NIBBLE_START
                movs    r1, #4                         @ counter
                movs    r2, #1                         @ PULL UP
                bl      init_multiple_pin_inputs

        pop     {r4, r5, pc}

#else @KEYBOARD_SERIAL_INTERFACE

/**
 * Initialize interrupts and data input for keyboard with PS-2 connector.
 *  r0 - keyboard DATA pin, the consecutive pin MUST be CLK pin
 */
.thumb_func
.global init_keyboard
.align 4
init_keyboard:
        push    {r4, lr}
        mov     r4, r0

        @ 0) Disable State Machine!!!
        movs    r0, #1                  @ PIO BLOCK 1
        movs    r1, #0                  @ State Machine 0
        movs    r2, #0                  @ disable
        bl      set_sm_enabled

        @ 1) Load program
        ldr     r0, =ps2_serial_rcv
        movs    r1, #10                 @ Total of 10 words
        movs    r2, #1                  @ PIO1
        bl      load_pio_prog

        @ 2) Set state machine exec ctrl with wrap (SM0_EXECCTRL); (WRAP_TARGET << 7) | (WRAP << 12)
        movs    r0, #1                  @ PIO BLOCK 1
        movs    r1, #0                  @ State Machine 0
        ldr     r2, =0xa000             @ bottom wrap at 0, top wrap at 10
        bl      setup_pio_execctrl

        @ 3) Set state machine pin ctrl
        mov     r2, r4
        lsls    r2, #15                 @ IN_BASE_BITS

        movs    r0, #1                  @ PIO BLOCK 1
        movs    r1, #0                  @ State Machine 0
        bl      setup_pio_pinctrl

        @ 4) setup autopush
        movs    r0, #1
        movs    r1, #0
        ldr     r2, =0x850000           @ IN_SHIFTDIR (18 bit), AUTOPUSH (16 bit), PUSH_THRESH (24:20 bits)
        bl      setup_pio_shiftctrl

        @ 5) setup interrupt
        movs    r0, #15                 @ IRQ15 = PIO0_IRQ_0
        ldr     r1, test_handler
        bl      set_isr

        @ 6) Setup pins
        @ According to the SDK:
        @       Note that, if your PIO program only needs the *input* from a given GPIO,
        @       it's not necessary to select the PIO GPIO function, because PIO input
        @       paths ignore the GPIO muxing. However, you must still configure the GPIO
        @       pad itself for input.
        mov     r0, r4
        movs    r1, #2                  @ two pins affected
        movs    r3, #1                  @ pull up
        bl      init_multiple_pin_inputs

        @ 7) PIO State Machine init - load and jump to first instruction
        movs    r0, #1
        movs    r1, #0
        movs    r2, #0                  @ address begin
        bl      clear_internal_and_jump

        @ 8) Set state machine running
        movs    r0, #1
        movs    r1, #0
        movs    r2, #1
        bl      set_sm_enabled

        pop     {r4, pc}

.thumb_func
.global test_handler
.align 4
test_handler:
        bkpt    #0
        bx      lr


.align 4
ps2_serial_rcv:
                        @       .wrap_target
        .word 0x2021    @ 0:            wait    0 pin, 1
        .word 0x20a1    @ 1:            wait    1 pin, 1
        .word 0xe027    @ 2:            set     x, 7 @ FIXME: PROPER OPCODE!!!
        .word 0x2021    @ 3:sample_data:wait    0 pin, 1
        .word 0x4001    @ 4:            in      pins, 1
        .word 0x20a1    @ 5:            wait    1 pin, 1
        .word 0x0043    @ 6:            jmp     x--, sample_data
        .word 0x2021    @ 7:            wait    0 pin, 1
        .word 0x20a1    @ 8:            wait    1 pin, 1
        .word 0x2021    @ 9:            wait    0 pin, 1
        @ .word 0x20a1    @ 10:           wait    1 pin, 1
        .word 0x22c1    @                       wait   1 irq, 1 [2]
                        @       .wrap

#endif



.thumb_func
.align 4
keyboard_interrupt_handler:
        cpsid   i
        push    {lr}

        @ 1) determine INTR_n
        ldr     r1, IO_BANK0_BASE
        ldr     r0, =interrupt_pin
        ldr     r0, [r0]
        lsrs    r0, r0, #3                              @ pin / 8 = INTR_n
        movs    r2, #0x04
        muls    r0, r0, r2
        ldr     r2, INTR_OFFSET
        add     r0, r0, r2
        add     r0, r0, r1

        @ 2) reset interrupt
        ldr     r1, =edge_high_bit
        ldr     r1, [r1]
        str     r1, [r0]                                @ clear by setting INTR

        @ 3) Proper handle
        ldr     r0, =keyboard_flags
        ldr     r0, [r0]
        ldr     r1, key_release_flag
        tst     r0, r1
        bne     .reset_flags

        ldr     r1, escape_flag
        tst     r0, r1
        bne     .manage_escape_sequence

        b       .read_key

        .reset_flags:
                ldr     r0, key_release_flag
                bl      clr_flag

                bl      read_byte
                ldr     r1, =scancode_read
                ldr     r1, [r1]
                cmp     r1, #0x12
                beq    .shift_released
                cmp     r1, #0x59
                beq     .shift_released

                cmp     r1, #0x14
                beq     .ctrl_released

                cmp     r1, #0x11
                beq     .alt_released

                b       .exit

        .read_key:
                bl      read_byte

                ldr     r0, =scancode_read
                ldr     r0, [r0]

                cmp     r0, #0xf0
                beq     .key_released

                cmp     r0, #0x12
                beq     .shift_pressed
                cmp     r0, #0x59
                beq     .shift_pressed
                cmp     r0, #0x58
                beq     .caps_lock_pressed

                cmp     r0, #0x14
                beq     .ctrl_pressed

                cmp     r0, #0x11
                beq     .alt_pressed

                cmp     r0, #0xe0
                beq     .escape_code_sequence_pressed

        .scancode_to_ascii:
                ldr     r1, =scancode_read
                ldrb    r0, [r1]

                ldr     r2, caps_lock_flag
                ldr     r3, shift_flag
                orrs    r3, r2, r3
                ldr     r2, =keyboard_flags
                ldr     r2, [r2]
                tst     r2, r3
                beq     .select_keymap_normal

                .select_shifted_keymap:
                        ldr     r1, =keymap_shifted
                        b       .convert_to_keymap
                .select_keymap_normal:
                        ldr     r1, =keymap
                .convert_to_keymap:
                        ldrb    r0, [r1, r0]

                ldr     r3, alt_flag
                tst     r2, r3
                beq     .write_to_buffer

@TODO: make version for ARCH_RP2040
#ifdef ARCH_RP2350
                @ umlauts
                cmp     r0, #65         @ 65->196, 97->228
                it      eq
                movseq  r0, #196

                cmp     r0, #97
                it      eq
                movseq  r0, #228

                cmp     r0, #79         @ 79->214, 111->246
                it      eq
                movseq  r0, #214

                cmp     r0, #111
                it      eq
                movseq  r0, #246

                cmp     r0, #85         @ 85->220, 117->252
                it      eq
                movseq  r0, #220

                cmp     r0, #117
                it      eq
                movseq  r0, #252

                cmp     r0, #115        @ 115 -> 223
                it      eq
                movseq  r0, #223

                @ e->EUR
                cmp     r0, #101        @ 101->128
                it      eq
                movseq  r0, #128

#endif

        .write_to_buffer:
                ldr     r1, =keyboard_buffer
                ldr     r2, =buffer_offset
                ldrb    r3, [r2]
                strb    r0, [r1, r3]
                adds    r3, r3, #1
                strb    r3, [r2]

                @ TODO: signal letter

                adds    r3, r3, r1
                subs    r0, r3, #1
                b       .exit_with_signal

        .manage_escape_sequence:
                ldr     r0, escape_flag
                bl      clr_flag

                bl      read_byte

                cmp     r0, #0xf0               @ is it key release?
                beq     .key_released

                @ RIGHT KEYS

                cmp     r0, #0x14               @ ctrl right
                beq     .ctrl_pressed
                cmp     r0, #0x11               @ alt right
                beq     .alt_pressed
                cmp     r0, #0x5a               @ enter
                beq     .scancode_to_ascii

                @ ARROWS

                cmp     r0, #0x72
                beq     .case_arrow_down
                cmp     r0, #0x74
                beq     .case_arrow_right
                cmp     r0, #0x75
                beq     .case_arrow_up
                cmp     r0, #0x6B
                beq     .case_arrow_left

                @ CTRL + KEY sequences

                b       .exit

        .case_arrow_up:
                movs    r0, ARROW_UP
                b       .write_escape_code_sequence

        .case_arrow_down:
                movs    r0, ARROW_DOWN
                b       .write_escape_code_sequence

        .case_arrow_left:
                movs    r0, ARROW_LEFT
                b       .write_escape_code_sequence

        .case_arrow_right:
                movs    r0, ARROW_RIGHT
                b       .write_escape_code_sequence

        .write_escape_code_sequence:
                ldr     r1, =keyboard_buffer
                ldr     r2, =buffer_offset
                ldrb    r2, [r2]

                movs    r3, #0x1b
                strb    r3, [r1, r2]
                adds    r2, r2, #1

                movs    r3, #0x5b
                strb    r3, [r1, r2]
                adds    r2, r2, #1

                strb    r0, [r1, r2]
                adds    r2, r2, #1

                ldr     r0, =buffer_offset
                strb    r2, [r0]


                @ TODO: signal escape code sequence

                adds    r2, r2, r1
                subs    r0, r2, #3
                b       .exit_with_signal

        .shift_pressed:
                ldr     r0, shift_flag
                bl      set_flag
                b       .exit

        .shift_released:
                ldr     r0, shift_flag
                bl      clr_flag
                b       .exit

        .caps_lock_pressed:
                ldr     r0, caps_lock_flag
                ldr     r1, =keyboard_flags
                ldr     r2, [r1]
                eors    r2, r2, r0
                str     r2, [r1]

                b       .exit

        .ctrl_pressed:

                ldr     r0, ctrl_flag
                bl      set_flag
                b       .exit

        .ctrl_released:
                ldr     r0, ctrl_flag
                bl      clr_flag
                b       .exit

        .alt_pressed:
                ldr     r0, alt_flag
                bl      set_flag
                b       .exit

        .alt_released:
                ldr     r0, alt_flag
                bl      clr_flag
                b       .exit


        .key_released:
                ldr     r0, key_release_flag
                bl      set_flag
                b       .exit

        .escape_code_sequence_pressed:
                ldr     r0, escape_flag
                bl      set_flag
                b       .exit

        .signal_special:
                @ SIGNALS LIKE ^C, ^D, ^Z

        .exit:
                cpsie   i
                pop     {pc}

        .exit_with_signal:
                cpsie   i
                movs    r1, IO_KEYBOARD
                bl      signal_resource
                pop     {pc}


/**
 * Sets keyboard flag
 *   r0 - flag to be set
 */
.thumb_func
.align 4
set_flag:
        ldr     r1, =keyboard_flags
        ldr     r2, [r1]
        orrs    r2, r2, r0
        str     r2, [r1]

        bx      lr

/**
 * Clears keyboard flag
 *   r0 - flag to be set
 */
.thumb_func
.align 4
clr_flag:
        ldr     r1, =keyboard_flags
        ldr     r2, [r1]
        eors    r2, r2, r0
        str     r2, [r1]

        bx      lr



.thumb_func
.align 4
read_byte:
        push    {r4, lr}
        ldr     r0, SIO_BASE
        adds    r0, r0, GPIO_IN
        ldr     r2, =scancode_read
        movs    r3, NIBBLE_MASK
        ldr     r4, =nibble_reverse_lookup

        .read_upper_nibble:
                ldr     r1, [r0]

                lsrs    r1, r1, UPPER_NIBBLE_START     @ move upper nibble to the right
                ands    r1, r1, r3                     @ leave upper nibble

                ldrb    r1, [r4, r1]                   @ lookup in table
                lsls    r1, r1, #4
                strb    r1, [r2]

        .read_lower_nibble:
                ldr     r1, [r0]

                lsrs    r1, r1, LOWER_NIBBLE_START     @ move lower
                ands    r1, r1, r3                     @ leave lower nibble

                ldrb    r1, [r4, r1]                   @ lookup in table
                ldrb    r3, [r2]
                add     r3, r3, r1
                strb    r3, [r2]

        mov     r0, r3
        pop     {r4, pc}


.thumb_func
.global clr_keyboard_buffer
.align 4
clr_keyboard_buffer:
        ldr     r0, =buffer_offset
        movs    r1, #0
        strb    r1, [r0]

        bx      lr


.thumb_func
.global keyboard_receive_char
.align 4
keyboard_receive_char:
        push    {r4}
        ldr     r1, =buffer_offset
        movs    r0, #0
        strb    r0, [r1]

        .wait_for_char:
                ldr     r1, =buffer_offset
                ldrb    r1, [r1]
                cmp     r0, r1
                beq     .wait_for_char

        subs    r1, r1, r0              @ how many bytes were written
        ldr     r2, =keyboard_buffer
        ldr     r3, =buffer_offset
        ldrb    r3, [r3]
        subs    r3, r3, r1              @ move offset to the beginning of written chars

        ldrb    r0, [r2, r3]

        .read_bytes_loop:
                adds    r3, r3, #1
                subs    r1, r1, #1
                cmp     r1, #0
                beq     .exit_reading_bytes

                lsls    r0, r0, #8
                ldrb    r4, [r2, r3]
                adds    r0, r0, r4

                b       .read_bytes_loop


        .exit_reading_bytes:
                pop     {r4}
                bx      lr


#ifdef ARCH_RP2040
.align 4
IO_BANK0_BASE:          .word 0x40014000
PADS_BANK0_BASE:        .word 0x4001c000
INTR_OFFSET:            .word 0x0f0
PROC0_INTE0_OFFSET:     .word 0x100

#elifdef ARCH_RP2350
IO_BANK0_BASE:          .word 0x40028000
PADS_BANK0_BASE:        .word 0x40038000
INTR_OFFSET:            .word 0x230
PROC0_INTE0_OFFSET:     .word 0x248

#endif

SIO_BASE:               .word 0xd0000000
PPB_BASE:               .word 0xe0000000
VTOR_OFFSET:            .word 0xed08
NVIC_ICPR_OFFSET:       .word 0xe280            @ interrupt clear-pending register
NVIC_ISER_OFFSET:       .word 0xe100            @ interrupt set-enable register

key_release_flag:       .word 0b000001
shift_flag:             .word 0b000010
ctrl_flag:              .word 0b000100
escape_flag:            .word 0b001000
alt_flag:               .word 0b010000
caps_lock_flag:         .word 0b100000

.align 4
nibble_reverse_lookup:  .byte 0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF


.section .data
.align 4
interrupt_pin:          .word 0
edge_high_bit:          .word 0
byte_read:              .word 0

scancode_read:          .word 0
keyboard_flags:         .word 0

.align 4
keyboard_buffer:        .fill 256, 1, 0
buffer_offset:          .byte 0                 @ when overflow starts again at 0

.section .rodata, "a"
.align 4
keymap:
        .ascii "?????????????"                  @ 00-0C
        .byte '\t'                              @ 0D = tab
        .ascii "`?"                             @ 0E-0F
        .ascii "?????q1???zsaw2?"               @ 10-1F
        .ascii "?cxde43?? vftr5?"               @ 20-2F
        .ascii "?nbhgy6???mju78?"               @ 30-3F
        .ascii "?,kio09??./l;p-?"               @ 40-4F
        .ascii "??'?[=????"                     @ 50-59
        .byte '\n'                              @ 5A
        .ascii "]?\\??"                         @ 5B-5F
        .ascii "??????"                         @ 60-65
        .byte '\b'                              @ 66
        .ascii "??1?47???"                      @ 67-6F
        .ascii "0.2568???+3-*9??"               @ 70-7F
        .ascii "????????????????"               @ 80-8F
        .ascii "????????????????"               @ 90-9F
        .ascii "????????????????"               @ A0-AF
        .ascii "????????????????"               @ B0-BF
        .ascii "????????????????"               @ C0-CF
        .ascii "????????????????"               @ D0-DF
        .ascii "????????????????"               @ E0-EF
        .byte 0xf0                              @ KEY RELEASE
        .ascii "???????????????"                @ F0-FF

.align 4
keymap_shifted:
        .ascii "?????????????"                  @ 00-0C
        .byte '\t'                              @ 0D = tab
        .ascii "`?"                             @ 0E-0F
        .ascii "?????Q!???ZSAW@?"               @ 10-1F
        .ascii "?CXDE$#?? VFTR%?"               @ 20-2F
        .ascii "?NBHGY^???MJU&*?"               @ 30-3F
        .ascii "?<KIO)(??>?L:P_?"               @ 40-4F
        .ascii "??"                             @ 50-51
        .byte '\"'                              @ 52
        .ascii "?{+????"                        @ 53-59
        .byte '\n'                              @ 5A
        .ascii "}?|??"                          @ 5B-5F
        .ascii "??????"                         @ 60-65
        .byte '\b'                              @ 66
        .ascii "??1?47???"                      @ 67-6F
        .ascii "0.2568???+3-*9??"               @ 70-7F
        .ascii "????????????????"               @ 80-8F
        .ascii "????????????????"               @ 90-9F
        .ascii "????????????????"               @ A0-AF
        .ascii "????????????????"               @ B0-BF
        .ascii "????????????????"               @ C0-CF
        .ascii "????????????????"               @ D0-DF
        .ascii "????????????????"               @ E0-EF
        .byte 0xf0                              @ KEY RELEASE
        .ascii "???????????????"                @ F0-FF
