#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb
.syntax unified

/**
 *
 */
.thumb_func
.global hsync_gen_init
.align 4
hsync_gen_init:
        push    {lr}
        @@@ According to squarewave example from SDK -
        @@@ pp. 313 (rp2040) / 877 (rp2350) in the datasheet.
        @@@
        @@@ The sequence of operations is taken straight from the SDK.

        @ 0) Disable State Machine!!!
        movs    r0, #0
        movs    r1, #0
        movs    r2, #0
        bl      set_sm_enabled

        @ 1) Load program
        ldr     r0, =hsync_gen
        movs    r1, #9                  @ Total of 9 words
        movs    r2, #0                  @ PIO0
        bl      load_pio_prog

        @ 2) Set state machine exec ctrl with wrap (SM0_EXECCTRL); (WRAP_TARGET << 7) | (WRAP << 12)
        movs    r0, #0
        movs    r1, #0
        ldr     r2, =0x8080             @ bottom wrap at 1, top wrap at 8
        bl      setup_pio_execctrl

        @ 3) Set state machine pin ctrl
        movs    r0, #1                  @ one pin affected
        lsls    r0, #26                 @ SET_COUNT_BITS
        movs    r1, #13                 @ HSYNC will be on pin 13
        lsls    r1, #5                  @ SET_BASE_BITS
        orrs    r2, r0, r1              @ control bits
        movs    r1, #0                  @ State Machine 0
        movs    r0, #0                  @ PIO BLOCK 0
        bl      setup_pio_pinctrl

        @ 4) Set clock divisor
        movs    r0, #0                  @ PIO BLOCK 0
        movs    r1, #0                  @ State Machine 0
        movs    r2, #5                  @ 125 MHz / 5 = 25 MHz
        lsls    r2, #16
        bl      setup_pio_clkdiv

        @ 5) Init GPIO
        movs    r0, #13                 @ GPIO 13
        movs    r1, #6                  @ Funcsel: PIO0
        bl      GPIO_function_select

        @ 6) Set consecutive pindirs to output
        movs    r0, #0
        movs    r1, #0
        movs    r2, #13
        movs    r3, #1
        bl      set_consecutive_pindirs_out

        @ 7) PIO State Machine init - load and jump to first instruction
        movs    r0, #0
        movs    r1, #0
        bl      clear_pio_sm_fifo

        movs    r0, #0
        movs    r1, #0
        movs    r2, #0                  @ address begin
        bl      clear_internal_and_jump

        @ 5) Set state machine running
        movs    r0, #0
        movs    r1, #0
        ldr     r2, =0x28f              @ 655 = 640 pixels + 16 frontporch - 1
        bl      sm_put

        movs    r0, #0
        movs    r1, #0
        movs    r2, #1
        bl      set_sm_enabled

        pop     {pc}




#ifdef VGA_640x480
/**
 * Generating Hsync signal. Total of 9 instructions.
 */
.align 4
hsync_gen:
        .word 0x80a0    @               pull   block
                        @       .wrap_target
        .word 0xa027    @               mov    x, osr
        .word 0x0042    @  activeporch: jmp    x--, activeporch
        .word 0xff00    @  pulse:       set    pins, 0 [31]
        .word 0xff00    @               set    pins, 0 [31]
        .word 0xff00    @               set    pins, 0 [31]
        .word 0xff01    @  backporch:   set    pins, 1 [31]
        .word 0xec01    @               set    pins, 1 [12]
        .word 0xc100    @               irq    0[1]
                        @       .wrap


/**
 * Generating Vsync signal. Total of 14 instructions.
 *  .side_set 1 opt
 */
.align 4
vsync_gen:
        .word 0x80a0    @               pull   block
                        @       .wrap_target
        .word 0xa027    @               mov    x, osr
        .word 0x20c0    @  active:      wait   1 irq, 0
        .word 0xc001    @               irq    1
        .word 0x0042    @               jmp    x--, active
        .word 0xe049    @               set    y, 9
        .word 0x20c0    @  frontporch:  wait   1 irq, 0
        .word 0x0086    @               jmp    y--, frontporch
        .word 0xe000    @  syncpulse:   set    pins, 0
        .word 0x20c0    @               wait   1 irq, 0
        .word 0x20c0    @               wait   1 irq, 0
        .word 0xe05f    @  backporch:   set    y, 31
        .word 0x38c0    @               wait   1 irq, 0        side 1
        .word 0x008c    @               jmp    y--, backporch
                        @       .wrap




#elifdef VGA_800x600
/**
 * Generating Hsync signal. Total of 10 instructions.
 */
.align 4
hsync_gen:
        .word 0x80a0    @               pull   block
                        @     .wrap_target
        .word 0xa027    @               mov    x, osr
        .word 0x0042    @  activeporch: jmp    x--, 2
        .word 0xe023    @               set    x, 3
        .word 0xff00    @  pulse:       set    pins, 0 [31]
        .word 0x0044    @               jmp    x--, pulse
        .word 0xff01    @  backporch:   set    pins, 1 [31]
        .word 0xff01    @               set    pins, 1 [31]
        .word 0xf401    @               set    pins, 1 [20]
        .word 0xc100    @               irq    0 [1]
                        @     .wrap

/**
 * Generating Vsync signal. Total of 12 instructions.
 *  .side_set 1 opt
 */
.align 4
vsync_gen:
        .wrod 0x80a0    @               pull   block
                        @       .wrap_target
        .word 0xa027    @               mov    x, osr
        .word 0x20c0    @  active:      wait   1 irq, 0
        .word 0xc001    @               irq    1
        .word 0x0042    @               jmp    x--, active
        .word 0x20c0    @  frontporch:  wait   1 irq, 0
        .word 0xe043    @               set    y, 3
        .word 0x30c0    @  syncpulse:   wait   1 irq, 0        side 0
        .word 0x0087    @               jmp    y--, syncpulse
        .word 0xe057    @               set    y, 23
        .word 0x38c0    @  backporch:   wait   1 irq, 0        side 1
        .word 0x008a    @               jmp    y--, 10
                        @       .wrap

#endif

/**
 * Generating RGB signals. Total of 8 instructions
 */
.align 4
rgb_gen:

        .word 0x80a0    @               pull   block
        .word 0xa047    @               mov    y, osr
                        @       .wrap_target
        .word 0xe000    @               set    pins, 0
        .word 0xa022    @               mov    x, y
        .word 0x22c1    @               wait   1 irq, 1               [2]
        .word 0x80a0    @  colorout:    pull   block
        .word 0x6006    @               out    pins, 6          @ for 6-bit mode
        @ .word 0x6003    @               out    pins, 3          @ for 3-bit mode
        .word 0x0045    @               jmp    x--, colorout
                        @       .wrap
