#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
.fpu fpv5-sp-d16
#endif
.thumb

.syntax unified

#include "syscalls.h"
#include "resources.h"

.equ USER_DEFINED_IVT_OFFSET, 0x40

.extern __heap_start__
.extern __heap_end__

.thumb_func
.global systick_enable
.align 4
systick_enable:
        ldr     r1, PPB_BASE
        ldr     r2, SYST_RVR
        subs    r0, r0, #1
        str     r0, [r1, r2]

        ldr     r2, SYST_CVR
        movs    r0, #0
        str     r0, [r1, r2]

        ldr     r2, SYST_CSR
        movs    r0, #7                  @ processor clock | systick exception | counter enabled
        str     r0, [r1, r2]

        bx      lr

.thumb_func
.global _exit
.align 4
_exit:
        bkpt    #0
        bx      lr

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
        mrs     r0, psp
        cmp     r0, #0
        beq     .switch_next_task

        mov     r2, lr                  @ EXC_RETURN
        mrs     r3, CONTROL
        stmdb   r0!, {r2-r11}

        msr     psp, r0                 @ save as a new stack pointer

.switch_next_task:
        bl      context_switch
        bkpt    #0                      @ will not return
        bx      lr

/**
 *
 */
.thumb_func
.global isr_systick
.align 4
isr_systick:
        @ ldr     r0, =next_task
        @ ldr     r1, [r0]
        @ cmp     r1, #0
        @ ite     eq
        @ movseq  r1, #1
        @ movsne  r1, #0
        @ str     r1, [r0]

        ldr     r0, PPB_BASE
        ldr     r1, ICSR
        ldr     r2, [r0, r1]
        movs    r3, #1
        lsls    r3, r3, #28
        orrs    r2, r2, r3
        str     r2, [r0, r1]

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


PPB_BASE:               .word 0xe0000000
SYST_CSR:               .word 0xe010
SYST_RVR:               .word 0xe014
ICSR:                   .word 0xed04
SYST_CVR:               .word 0xe018
VTOR_OFFSET:            .word 0xed08
NVIC_ICPR_OFFSET:       .word 0xe280        @ interrupt clear-pending register
NVIC_ISER_OFFSET:       .word 0xe100        @ interrupt set-enable register

.section .data
next_task:              .word 0
