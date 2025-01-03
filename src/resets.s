.cpu cortex-m0
.thumb

.syntax unified

.equ RESET_DONE_OFFSET, 0x08

/**
 * Resets GPIO, PADS and UART
 *
 */
.thumb_func
.global reset_subsys
.align 4
reset_subsys:
    push {lr}
    @ 1) Reset GPIO
    movs r0, #32                        @ Bit 5 - IO_BANK0
    bl   hw_reset

    @ 2) Reset PADS
    movs r0, #1
    lsls r0, #8                         @ Bit 8 - PADS_BANK0
    bl   hw_reset

    @ 3) Reset UART
    movs r0, #1
    lsls r0, #22                        @ Bit 22 - UART0
    bl   hw_reset

    pop  {pc}


/**
 * Resets peripherial specified in r0.
 */
.thumb_func
.align 4
hw_reset:
    ldr  r2, RESETS_BASE
    ldr  r1, ATOMIC_BITMASK_CLR         @ Atomic register access
    add  r1, r1, r2
    str  r0, [r1]

    adds r2, r2, RESET_DONE_OFFSET

    .reset:
        ldr  r1, [r2]
        tst  r1, r0
        beq  .reset

    bx   lr


.align 4
RESETS_BASE:        .word 0x4000c000
ATOMIC_BITMASK_CLR: .word 0x3000
