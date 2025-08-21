#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb

.syntax unified

.equ RESTART_SVC,     0
.equ EXIT_SVC,        1
.equ FORK_SVC,        2
.equ READ_SVC,        3
.equ WRITE_SVC,       4
.equ OPEN_SVC,        5
.equ CLOSE_SVC,       6
.equ WAIT_PID_SVC,    7
.equ GET_TIME_SVC,    8
.equ GET_PID_SVC,     9
.equ GET_PPID_SVC,    10
.equ YIELD_SVC,       11
.equ KILL,            12

.equ USER_DEFINED_IVT_OFFSET, 0x40

/**
 *
 */
.thumb_func
.global isr_svcall
.align 4
isr_svcall:
        cmp     r7, YIELD_SVC
        bx      lr



/**
 *
 */
.thumb_func
.global isr_pendsv
.align 4
isr_pendsv:

        bx      lr

/**
 *
 */
.thumb_func
.global isr_systick
.align 4
isr_systick:

        bx      lr


/**
 * Writes N-th interrupt handler to the IVT. FIXME: currently only for irq 0-31
 *  r0 - interrupt number
 *  r1 - interrupt handler
 * */
.thumb_func
.global set_isr
.align 4
set_isr:
        ldr     r2, PPB_BASE
        ldr     r3, VTOR_OFFSET
        ldr     r2, [r2, r3]               @ read the address of IVT from VTOR hardware register

        movs    r3, #4
        muls    r3, r0, r3
        adds    r3, r3, USER_DEFINED_IVT_OFFSET
        str     r1, [r2, r3]               @ write interrupt handler

        @@@@ enable appropriate interrupt at the CPU by turning off and on again @@@@

        movs    r1, #1
        lsls    r0, r1, r0

        ldr     r1, PPB_BASE
        ldr     r2, NVIC_ICPR_OFFSET       @ unset IRQ
        str     r0, [r1, r2]
        ldr     r2, NVIC_ISER_OFFSET       @ set IRQ
        str     r0, [r1, r2]

        bx      lr


PPB_BASE:           .word 0xe0000000
VTOR_OFFSET:        .word 0xed08
NVIC_ICPR_OFFSET:   .word 0xe280        @ interrupt clear-pending register
NVIC_ISER_OFFSET:   .word 0xe100        @ interrupt set-enable register
