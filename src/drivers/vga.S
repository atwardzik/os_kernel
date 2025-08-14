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

        movs    r0, #0                  @ PIO BLOCK 0
        movs    r1, #0                  @ State Machine 0
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

        @ 8) Set state machine running
        movs    r0, #0
        movs    r1, #0
        ldr     r2, =0x28f              @ 655 = 640 pixels + 16 frontporch - 1
        bl      sm_put

        movs    r0, #0
        movs    r1, #0
        movs    r2, #1
        @ bl      set_sm_enabled

        pop     {pc}



.thumb_func
.global vsync_gen_init
.align 4
vsync_gen_init:
        push    {lr}

        @ 0) Disable State Machine!!!
        movs    r0, #0
        movs    r1, #1
        movs    r2, #0
        bl      set_sm_enabled

        @ 1) Load program
        ldr     r0, =vsync_gen
        movs    r1, #14                 @ Total of 14 words
        movs    r2, #0                  @ PIO0
        bl      load_pio_prog

        @ 2) Set state machine exec ctrl with wrap (SM0_EXECCTRL); (WRAP_TARGET << 7) | (WRAP << 12)
        movs    r0, #0
        movs    r1, #1
        ldr     r2, =0x16500            @ bottom wrap at 1, top wrap at 14 + OFFSET
        @ add info about SIDE_EN
        movs    r3, #1
        lsls    r3, r3, #30             @ SIDE_EN bit
        orrs    r2, r2, r3
        bl      setup_pio_execctrl

        @ 3) Set state machine pin ctrl
        @ add info about SET_COUNT
        movs    r0, #1                  @ one pin affected
        lsls    r0, #26                 @ SET_COUNT_BITS

        @ add info about SET_BASE
        movs    r1, #14                 @ VSYNC will be on pin 14
        lsls    r1, #5                  @ SET_BASE_BITS
        orrs    r2, r0, r1              @ control bits

        @ add info about SIDESET_COUNT
        movs    r0, #2                  @ two pins affected (inclusive the enable bit)
        lsls    r0, #29                 @ SIDESET_COUNT_BITS
        orrs    r2, r2, r0

        @ add info about SIDESET_BASE
        movs    r0, #14                 @ VSYNC will be on pin 14
        lsls    r0, #10                 @ SIDESET_BASE_BITS
        orrs    r2, r2, r0              @ control bits

        movs    r0, #0                  @ PIO BLOCK 0
        movs    r1, #1                  @ State Machine 1
        bl      setup_pio_pinctrl

        @ 4) Set clock divisor
        movs    r0, #0                  @ PIO BLOCK 0
        movs    r1, #1                  @ State Machine 1
        movs    r2, #5                  @ 125 MHz / 5 = 25 MHz
        lsls    r2, #16
        bl      setup_pio_clkdiv

        @ 5) Init GPIO
        movs    r0, #14                 @ GPIO 14
        movs    r1, #6                  @ Funcsel: PIO0
        bl      GPIO_function_select

        @ 6) Set consecutive pindirs to output
        movs    r0, #0
        movs    r1, #1
        movs    r2, #14
        movs    r3, #1
        bl      set_consecutive_pindirs_out

        @ 7) PIO State Machine init - load and jump to first instruction
        movs    r0, #0
        movs    r1, #1
        bl      clear_pio_sm_fifo

        movs    r0, #0
        movs    r1, #1
        movs    r2, #9                  @ address begin
        bl      clear_internal_and_jump

        @ 8) Set state machine running
        movs    r0, #0
        movs    r1, #1
        ldr     r2, =0x1df              @ 479 = 480 pixels - 1
        bl      sm_put

        movs    r0, #0
        movs    r1, #1
        movs    r2, #1
        @ bl      set_sm_enabled

        pop     {pc}


/**
 *
 */
.thumb_func
.global rgb_gen_init
.align 4
rgb_gen_init:
        push    {lr}
        @ 0) Disable State Machine!!!
        movs    r0, #0
        movs    r1, #2
        movs    r2, #0
        bl      set_sm_enabled

        @ 1) Load program
        ldr     r0, =rgb_gen
        movs    r1, #8                  @ Total of 8 words
        movs    r2, #0                  @ PIO0
        bl      load_pio_prog

        @ 2) Set state machine exec ctrl with wrap (SM0_EXECCTRL); (WRAP_TARGET << 7) | (WRAP << 12)
        movs    r0, #0
        movs    r1, #2
        ldr     r2, =0x1ec80            @ bottom wrap at 1, top wrap at 8
        bl      setup_pio_execctrl

        @ 3) Set state machine pin ctrl
        movs    r0, #5                  @ one pin affected
        lsls    r0, #26                 @ SET_COUNT_BITS
        movs    r1, #16                 @ RGB will start on pin 16
        lsls    r1, #5                  @ SET_BASE_BITS
        orrs    r2, r0, r1              @ control bits
        ldr     r0, =0x500010           @ OUT_COUNT_BITS and OUT_BASE_BITS
        orrs    r2, r2, r0

        movs    r0, #0                  @ PIO BLOCK 0
        movs    r1, #2                  @ State Machine 0
        bl      setup_pio_pinctrl

        @ 4) Set clock divisor
        @movs    r0, #0                  @ PIO BLOCK 0
        @movs    r1, #2                  @ State Machine 0
        @movs    r2, #1                  @ 125 MHz / 1 = 125 MHz
        @lsls    r2, #16
        @bl      setup_pio_clkdiv

        @ 5) Init GPIO
        movs    r0, #16                 @ GPIO 16
        movs    r1, #6                  @ Funcsel: PIO0
        bl      GPIO_function_select

        movs    r0, #17                 @ GPIO 17
        movs    r1, #6                  @ Funcsel: PIO0
        bl      GPIO_function_select

        movs    r0, #18                 @ GPIO 18
        movs    r1, #6                  @ Funcsel: PIO0
        bl      GPIO_function_select

        movs    r0, #19                 @ GPIO 19
        movs    r1, #6                  @ Funcsel: PIO0
        bl      GPIO_function_select

        movs    r0, #20                 @ GPIO 20
        movs    r1, #6                  @ Funcsel: PIO0
        bl      GPIO_function_select

        @ 6) Set consecutive pindirs to output
        movs    r0, #0
        movs    r1, #2
        movs    r2, #16
        movs    r3, #5
        bl      set_consecutive_pindirs_out

        @ 7) PIO State Machine init - load and jump to first instruction
        movs    r0, #0
        movs    r1, #2
        bl      clear_pio_sm_fifo

        movs    r0, #0
        movs    r1, #2
        movs    r2, #23                 @ address begin
        bl      clear_internal_and_jump

        @ 8) Set state machine running
        movs    r0, #0
        movs    r1, #2
        ldr     r2, =0x27f              @ 639 = 640 pixels - 1
        bl      sm_put

        movs    r0, #0
        movs    r1, #2
        movs    r2, #1
        @ bl      set_sm_enabled

        pop     {pc}

.thumb_func
.global setup_vga_dma
.align 4
setup_vga_dma:
        @ CH0
        ldr     r0, =0x50000000         @ CH0_READ_ADDR
        ldr     r1, =0x20035000         @ address start
        str     r1, [r0]

        adds    r0, r0, #4              @ CH0_WRITE_ADDR
        ldr     r1, =0x50200018         @ rgb pio tx fifo
        str     r1, [r0]

        adds    r0, r0, #4              @ CH0_TRANSFER_COUNT
        ldr     r1, =307200             @ vidram size
        str     r1, [r0]

        adds    r0, r0, #8              @ CH0_AL1_CTRL
        ldr     r1, =0x42011
        str     r1, [r0]

        @ CH1
        ldr     r0, =0x50000040         @ CH1_READ_ADDR
        ldr     r1, =VIDRAM_START       @ TODO: CHANGE TO BE LINKER DEPENDENT
        str     r1, [r0]

        adds    r0, r0, #4              @ CH1_WRITE_ADDR
        ldr     r1, =0x50000000         @ CH0_READ_ADDR
        str     r1, [r0]

        adds    r0, r0, #4              @ CH1_TRANSFER_COUNT
        movs    r1, #1                  @ vidram size
        str     r1, [r0]

        adds    r0, r0, #8              @ CH1_AL1_CTRL
        ldr     r1, =0x7e0009
        str     r1, [r0]

        bx      lr

.align 4
VIDRAM_START:   .word 0x20035000


#ifdef VGA_640x480
/**
 * Generating Hsync signal. Total of 9 instructions.
 */
.align 4
hsync_gen:
        .word 0x80a0    @ 0:            pull   block
                        @       .wrap_target
        .word 0xa027    @ 1:            mov    x, osr
        .word 0x0042    @ 2:activeporch:jmp    x--, activeporch
        .word 0xff00    @ 3: pulse:     set    pins, 0 [31]
        .word 0xff00    @ 4:            set    pins, 0 [31]
        .word 0xff00    @ 5:            set    pins, 0 [31]
        .word 0xff01    @ 6: backporch: set    pins, 1 [31]
        .word 0xec01    @ 7:            set    pins, 1 [12]
        .word 0xc100    @ 8:            irq    0[1]
                        @       .wrap


/**
 * Generating Vsync signal. Total of 14 instructions.
 *  .side_set 1 opt
 */
.align 4
vsync_gen:
        .word 0x80a0    @ 9:            pull   block
                        @       .wrap_target
        .word 0xa027    @ 10:           mov    x, osr
        .word 0x20c0    @ 11: active:   wait   1 irq, 0
        .word 0xc001    @ 12:           irq    1
        .word 0x004b    @ 13:           jmp    x--, active
        .word 0xe049    @ 14:           set    y, 9
        .word 0x20c0    @ 15:frontporch:wait   1 irq, 0
        .word 0x008f    @ 16:           jmp    y--, frontporch
        .word 0xe000    @ 17:syncpulse: set    pins, 0
        .word 0x20c0    @ 18:           wait   1 irq, 0
        .word 0x20c0    @ 19:           wait   1 irq, 0
        .word 0xe05f    @ 20:           set    y, 31
        .word 0x38c0    @ 21:backporch  wait   1 irq, 0        side 1
        .word 0x0095    @ 22:           jmp    y--, backporch
                        @       .wrap

/**
 * Generating RGB signals. Total of 8 instructions. Running at 125[MHz]
 */
.align 4
rgb_gen:

        .word 0x80a0    @ 23:           pull   block
        .word 0xa047    @ 24:           mov    y, osr
                        @       .wrap_target
        .word 0xe000    @ 25:           set    pins, 0
        .word 0xa022    @ 26:           mov    x, y
        .word 0x23c1    @ 27:           wait   1 irq, 1 [3]
        @ .word 0xa042
        @ .word 0xe203
        .word 0x80a0    @ 28: colorout: pull   block
        .word 0x6205    @ 29:           out    pins, 5  [2]      @ for 5-bit mode
        @ .word 0x6203    @ 29:           out    pins, 3 [2]       @ for 3-bit mode
        .word 0x005c    @ 30:           jmp    x--, colorout
                        @       .wrap

/*
 * TODO: fix and check in future
 */
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

.section .vidram
.incbin "transformed.raw"
