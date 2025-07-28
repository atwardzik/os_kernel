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

        bx lr




#ifdef VGA_640x480
/*
 * Generating Hsync signal. Total of 9 instructions.
 */
.align 4
hsync_gen:
        .word 0x80a0,   @               pull   block
                        @       .wrap_target
        .word 0xa027,   @               mov    x, osr
        .word 0x0042,   @  activeporch: jmp    x--, activeporch
        .word 0xff00,   @  pulse:       set    pins, 0 [31]
        .word 0xff00,   @               set    pins, 0 [31]
        .word 0xff00,   @               set    pins, 0 [31]
        .word 0xff01,   @  backporch:   set    pins, 1 [31]
        .word 0xec01,   @               set    pins, 1 [12]
        .word 0xc100,   @               irq    0[1]
                        @       .wrap


/*
 * Generating Vsync signal. Total of 14 instructions.
 *  .side_set 1 opt
 */
.align 4
vsync_gen:
        .word 0x80a0,   @               pull   block
                        @       .wrap_target
        .word 0xa027,   @               mov    x, osr
        .word 0x20c0,   @  active:      wait   1 irq, 0
        .word 0xc001,   @               irq    1
        .word 0x0042,   @               jmp    x--, active
        .word 0xe049,   @               set    y, 9
        .word 0x20c0,   @  frontporch:  wait   1 irq, 0
        .word 0x0086,   @               jmp    y--, frontporch
        .word 0xe000,   @  syncpulse:   set    pins, 0
        .word 0x20c0,   @               wait   1 irq, 0
        .word 0x20c0,   @               wait   1 irq, 0
        .word 0xe05f,   @  backporch:   set    y, 31
        .word 0x38c0,   @               wait   1 irq, 0        side 1
        .word 0x008c,   @               jmp    y--, backporch
                        @       .wrap




#elifdef VGA_800x600
/*
 * Generating Hsync signal. Total of 10 instructions.
 */
.align 4
hsync_gen:
        .word 0x80a0,   @               pull   block
                        @     .wrap_target
        .word 0xa027,   @               mov    x, osr
        .word 0x0042,   @  activeporch: jmp    x--, 2
        .word 0xe023,   @               set    x, 3
        .word 0xff00,   @  pulse:       set    pins, 0 [31]
        .word 0x0044,   @               jmp    x--, pulse
        .word 0xff01,   @  backporch:   set    pins, 1 [31]
        .word 0xff01,   @               set    pins, 1 [31]
        .word 0xf401,   @               set    pins, 1 [20]
        .word 0xc100,   @               irq    0 [1]
                        @     .wrap

/*
 * Generating Vsync signal. Total of 12 instructions.
 *  .side_set 1 opt
 */
.align 4
vsync_gen:
        .wrod 0x80a0,   @               pull   block
                        @       .wrap_target
        .word 0xa027,   @               mov    x, osr
        .word 0x20c0,   @  active:      wait   1 irq, 0
        .word 0xc001,   @               irq    1
        .word 0x0042,   @               jmp    x--, active
        .word 0x20c0,   @  frontporch:  wait   1 irq, 0
        .word 0xe043,   @               set    y, 3
        .word 0x30c0,   @  syncpulse:   wait   1 irq, 0        side 0
        .word 0x0087,   @               jmp    y--, syncpulse
        .word 0xe057,   @               set    y, 23
        .word 0x38c0,   @  backporch:   wait   1 irq, 0        side 1
        .word 0x008a,   @               jmp    y--, 10
                        @       .wrap

#endif

/*
 * Generating RGB signals. Total of 8 instructions
 */
.align 4
rgb_gen:

        .word 0x80a0,   @               pull   block
        .word 0xa047,   @               mov    y, osr
                        @       .wrap_target
        .word 0xe000,   @               set    pins, 0
        .word 0xa022,   @               mov    x, y
        .word 0x22c1,   @               wait   1 irq, 1               [2]
        .word 0x80a0,   @  colorout:    pull   block
        .word 0x6006,   @               out    pins, 6
        .word 0x0045,   @               jmp    x--, colorout
                        @       .wrap
