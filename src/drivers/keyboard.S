#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified


.equ GPIO_IN, 0x04

@ Please note that the pins are consecutive, e.g. when starting at pin 2, pins {2,3,4,5} will be treaded as input
.equ UPPER_NIBBLE_START, 2
.equ LOWER_NIBBLE_START, 6
.equ NIBBLE_MASK, 0x0F


#ifdef ARCH_RP2040
.equ IO_IRQ_BANK0, 13
#elifdef ARCH_RP2350
.equ IO_IRQ_BANK0, 21
#endif


/**
 * Initialize interrupts and data input for keyboard with PS-2 connector.
 * r0 - keyboard interrupt pin
 */
.thumb_func
.global init_keyboard
.align 4
init_keyboard:
        push {lr}

        ldr  r1, =interrupt_pin
        str  r0, [r1]

        @ determine bit for edge high -> (pin % 8) * 4 + 3
        ldr  r0, =edge_high_bit
        ldr  r1, interrupt_pin
        movs r2, #0x07
        ands r1, r1, r2                         @ equivalent to (pin % 8)

        movs r2, #0x04
        muls r1, r1, r2
        adds r1, r1, #0x03

        movs r2, #1
        lsls r2, r2, r1

        str  r2, [r0]

        .init_pin_interrupt:
                @ 1) GPIO INIT
                ldr  r0, interrupt_pin
                movs r1, #5                         @ 5 - SIO
                bl   GPIO_function_select

                @ 2) CONFIGURATION
                ldr  r0, interrupt_pin
                movs r1, #1                         @ PULL UP
                bl   init_pin_input_with_pull

                @ 3) GPIO_SET_IRQ_ENABLED
                @ 3.1) acknowledge_irq - "clear stale events which might cause immediate spurious handler entry"
                ldr  r1, IO_BANK0_BASE

                ldr  r0, interrupt_pin
                lsrs r0, r0, #3                     @ pin / 8 = INTR_n
                movs r2, #0x04
                muls r0, r0, r2
                ldr  r3, INTR_OFFSET
                add  r0, r0, r3
                add  r0, r0, r1

                ldr  r1, edge_high_bit
                str  r1, [r0]                       @ clear by setting INTR

                @ 3.2) set irq enabled
                ldr  r0, IO_BANK0_BASE

                ldr  r1, interrupt_pin
                lsrs r1, r1, #3                     @ pin / 8 = INTE_n
                movs r2, #0x04
                muls r1, r1, r2

                ldr  r2, PROC0_INTE0_OFFSET
                add  r1, r1, r2
                add  r0, r0, r1

                ldr  r1, edge_high_bit
                str  r1, [r0]                       @ set irq enabled - INTE

                @ 4) set interrupt subroutine
                movs    r0, IO_IRQ_BANK0
                ldr     r1, =pin_handler
                bl      set_isr

        .init_keyboard_input_pins:
                .init_upper_word:
                movs r0, UPPER_NIBBLE_START
                movs r1, #4                         @ counter
                movs r2, #1                         @ PULL UP
                bl   init_multiple_pin_inputs

                .init_lower_word:
                movs r0, LOWER_NIBBLE_START
                movs r1, #4                         @ counter
                movs r2, #1                         @ PULL UP
                bl   init_multiple_pin_inputs

        pop {pc}


.thumb_func
.align 4
pin_handler:
        cpsid i
        push {lr}

        @ 1) determine INTR_n
        ldr  r1, IO_BANK0_BASE
        ldr  r0, interrupt_pin
        lsrs r0, r0, #3                         @ pin / 8 = INTR_n
        movs r2, #0x04
        muls r0, r0, r2
        ldr  r2, INTR_OFFSET
        adds r0, r0, r2
        add  r0, r0, r1

        @ 2) reset interrupt
        ldr  r1, edge_high_bit
        str  r1, [r0]                           @ clear by setting INTR

        @@@@@@@@ do some stuff @@@@@@@@@@
        @ movs r0, #25
        @ bl   xor_pin

        ldr  r0, keyboard_flags
        ldr  r1, key_release_flag
        tst  r0, r1
        beq  .read_key

        .reset_flags:
                eors r0, r0, r1                         @ reset release flag
                ldr  r1, =keyboard_flags
                str  r0, [r1]

                bl   read_byte
                ldr  r1, =scancode_read
                ldr  r1, [r1]
                cmp  r1, #0x12
                beq  .shift_released
                cmp  r1, #0x59
                beq  .shift_released

                b    .exit

        .read_key:
                bl   read_byte

                ldr  r1, =scancode_read
                ldr  r1, [r1]

                cmp  r1, #0xf0
                beq  .key_released
                cmp  r1, #0x12
                beq  .shift_pressed
                cmp  r1, #0x59
                beq  .shift_pressed

        .scancode_to_ascii:
                ldr  r1, =scancode_read
                ldrb r0, [r1]

                ldr  r2, keyboard_flags
                ldr  r3, shift_flag
                tst  r2, r3
                beq  .select_keymap_normal
                .select_shifted_keymap:
                        ldr  r1, =keymap_shifted
                        b    .convert_to_keymap
                .select_keymap_normal:
                        ldr  r1, =keymap
                .convert_to_keymap:
                        ldrb r0, [r1, r0]

        .write_to_buffer:
                ldr  r1, =keyboard_buffer
                ldr  r2, =buffer_offset
                ldrb r3, [r2]
                strb r0, [r1, r3]
                adds r3, r3, #1
                strb r3, [r2]
                b    .exit

        .shift_pressed:
                ldr  r0, =keyboard_flags
                ldr  r1, shift_flag
                ldr  r2, [r0]
                orrs r2, r2, r1
                str  r2, [r0]

                b    .exit

        .shift_released:
                ldr  r0, =keyboard_flags
                ldr  r1, shift_flag
                ldr  r2, [r0]
                eors r2, r2, r1
                str  r2, [r0]

                b    .exit

        .key_released:
                ldr  r0, =keyboard_flags
                ldr  r1, key_release_flag
                ldr  r2, [r0]
                orrs r2, r2, r1
                str  r2, [r0]

        .exit:
                cpsie i
                pop {pc}



.thumb_func
.align 4
read_byte:
        push {r4, lr}
        ldr  r0, SIO_BASE
        adds r0, r0, GPIO_IN
        ldr  r2, =scancode_read
        movs r3, NIBBLE_MASK
        ldr  r4, =nibble_reverse_lookup

        .read_upper_nibble:
                ldr  r1, [r0]

                lsrs r1, r1, UPPER_NIBBLE_START     @ move upper nibble to the right
                ands r1, r1, r3                     @ leave upper nibble

                ldrb r1, [r4, r1]                   @ lookup in table
                lsls r1, r1, #4
                strb r1, [r2]

        .read_lower_nibble:
                ldr  r1, [r0]

                lsrs r1, r1, LOWER_NIBBLE_START     @ move lower
                ands r1, r1, r3                     @ leave lower nibble

                ldrb r1, [r4, r1]                   @ lookup in table
                ldrb r3, [r2]
                add  r3, r3, r1
                strb r3, [r2]

        pop {r4, pc}

.thumb_func
.global keyboard_receive_char
.align 4
keyboard_receive_char:
        ldr  r0, =buffer_offset
        ldrb r0, [r0]

        .wait_for_char:
                ldr  r1, =buffer_offset
                ldrb r1, [r1]
                cmp  r0, r1
                beq  .wait_for_char

        ldr  r0, =keyboard_buffer
        ldr  r1, =buffer_offset
        ldrb r1, [r1]
        subs r1, r1, #1
        ldrb r0, [r0, r1]

        bx   lr


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

SIO_BASE:           .word 0xd0000000
PPB_BASE:           .word 0xe0000000
VTOR_OFFSET:        .word 0xed08
NVIC_ICPR_OFFSET:   .word 0xe280            @ interrupt clear-pending register
NVIC_ISER_OFFSET:   .word 0xe100            @ interrupt set-enable register

.align 4
nibble_reverse_lookup: .byte 0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF

// section data?
.align 4
interrupt_pin:      .word 0
edge_high_bit:      .word 0
byte_read:          .word 0

.align 4
scancode_read:      .word 0
buffer_offset:      .byte 0                 @ when overflow starts again at 0

.align 4
keyboard_flags:     .word 0
key_release_flag:   .word 0b0001
shift_flag:         .word 0b0010

.align 4
keyboard_buffer: .fill 256, 1, 0

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
