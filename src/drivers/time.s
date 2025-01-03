.cpu cortex-m0
.thumb
.syntax unified



.equ ALARM0_IVT_OFFSET, 0x40
.equ IO_IRQ_BANK0_OFFSET, 0x74

.equ TIMER_INTE, 0x38
.equ TIMER_INTR, 0x34
.equ TIMER_TIMERAWL_OFFSET, 0x28
.equ TIMER_ALARM0_OFFSET, 0x10

.equ XOSC_CTRL, 0x00
.equ XOSC_STATUS, 0x04
.equ XOSC_STARTUP, 0x0c
.equ STARTUP_DELAY, 0xc4


.equ CLK_REF_CTRL, 0x30
.equ XOSC_CLKSRC, 0x02

.equ CLK_REF_DIV, 0x34
.equ CLK_SYS_CTRL, 0x3c
.equ CLK_PERI_CTRL, 0x48



.thumb_func
.global setup_internal_clk
.align 4
setup_internal_clk:
    .init_watchdog:
        ldr  r0, WATCHDOG_TICK
        movs r1, #1
        lsls r1, r1, #9                 @ watchdog tick enable bits
        movs r2, #12                    @ cycles = 12 [MHz]
        orrs r1, r1, r2                 @ cycles | WATCHDOG_TICK_ENABLE_BITS
        str  r1, [r0]

    .enable_xosc:
        ldr  r0, XOSC_BASE

        @ 1) Set frequency range
        movs r1, XOSC_CTRL
        adds r1, r1, r0

        ldr  r2, FREQ_RANGE
        str  r2, [r1]

        @ 2) Startup delay
        movs r1, XOSC_STARTUP
        adds r1, r1, r0

        movs r2, STARTUP_DELAY
        str  r2, [r1]

        @ 3) Enable crystall oscillator
        ldr  r1, ATOMIC_BITMASK_SET
        adds r1, r1, r0

        ldr  r2, XOSC_ENABLE
        str  r2, [r1]

        @ 4) Wait for oscillating signal stabilization
        ldr  r2, XOSC_STABLE
        movs r1, XOSC_STATUS
        adds r1, r1, r0

        .wait_for_xosc:
            ldr r3, [r1]
            tst r3, r2
            beq .wait_for_xosc

    .switch_clocks_to_xosc:
        ldr  r0, CLOCKS_BASE

        @ 1) CLK REF source = xosc_clksrc
        movs r1, CLK_REF_CTRL
        adds r1, r1, r0

        movs r2, XOSC_CLKSRC
        str  r2, [r1]

        @ 3) CLK SYS source = clk_ref
        movs r1, CLK_SYS_CTRL
        adds r1, r1, r0

        movs r2, #0                     @ 0 - clk_ref
        str  r2, [r1]

        @ 2) CLK REF Divisor = 1
        movs r1, CLK_REF_DIV
        adds r1, r1, r0

        movs r2, #1
        lsls r2, r2, #8
        str  r2, [r1]

        @ 4) CLK PERI
        movs r1, CLK_PERI_CTRL
        adds r1, r1, r0

        movs r2, #1
        lsls r2, r2, #11                @ ENABLE - Starts and stops the clock generator cleanly

        movs r3, #4
        lsls r3, r3, #5                 @ Bits 7:5 - 0x4 -> xosc_clksrc

        orrs r2, r2, r3
        str  r2, [r1]

    bx   lr


.align 4
XOSC_BASE:          .word 0x40024000
FREQ_RANGE:         .word 0xaa0
XOSC_ENABLE:        .word 0xfab000
ATOMIC_BITMASK_SET: .word 0x2000
XOSC_STABLE:        .word 0x80000000

CLOCKS_BASE:        .word 0x40008000
WATCHDOG_TICK:      .word 0x4005802c


/**
 * Delays by the amount of microseconds specified in r0.
 * */
.thumb_func
.global delay_ms
.align 4
delay_ms:
    push {lr}

    ldr  r1, =1000
    muls r0, r0, r1
    bl   delay_us

    pop  {pc}


/**
 * Delays by the amount of microseconds specified in r0.
 * */
.thumb_func
.global delay_us
.align 4
delay_us:
    ldr  r3, TIMER_BASE
    ldr  r1, [r3, TIMER_TIMERAWL_OFFSET]        @ get TIMERAWL
    add  r1, r1, r0                             @ TIMERAWL + desired alarm

    .delay_loop:
        ldr r2, [r3, TIMER_TIMERAWL_OFFSET]
        cmp r2, r1
        blt .delay_loop

    bx  lr


/**
 * Writes alarm0 interrupt handler to the IVT.
 * */
.thumb_func
.global set_alarm0_isr
.align 4
set_alarm0_isr:
    ldr  r2, PPB_BASE
    ldr  r1, VTOR_OFFSET
    add  r2, r2, r1
    ldr  r1, [r2]                   @ read the address of IVT from VTOR hardware register

    movs r2, ALARM0_IVT_OFFSET
    add  r2, r2, r1
    ldr  r0, =alarm0_handler
    str  r0, [r2]                   @ write interrupt handler

    @@@@ enable appropriate interrupt at the CPU by turning off and on again @@@@

    movs r0, #1                     @ alarm 0 is irq0
    ldr  r2, PPB_BASE
    ldr  r1, NVIC_ICPR_OFFSET       @ unset IRQ
    add  r1, r2
    str  r0, [r1]
    ldr  r1, NVIC_ISER_OFFSET       @ set IRQ
    add  r1, r2
    str  r0, [r1]

    bx   lr


.thumb_func
.align 4
alarm0_handler:
    ldr  r3, TIMER_BASE
    adds r3, r3, TIMER_INTR
    movs r2, #1
    str  r2, [r3]                   @ reset interrupt for alarm0

    ldr  r3, =alarm_fired
    movs r2, #1                     @ alarm_fired = True
    strb r2, [r3]

    bx  lr


.align 4
alarm_fired:        .byte 0

.align 4
TIMER_BASE:         .word 0x40054000
PPB_BASE:           .word 0xe0000000
VTOR_OFFSET:        .word 0xed08
NVIC_ICPR_OFFSET:   .word 0xe280        @ interrupt clear-pending register
NVIC_ISER_OFFSET:   .word 0xe100        @ interrupt set-enable register
