#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
.fpu fpv5-sp-d16
#endif
.thumb

.syntax unified

#include "syscalls.h"

.equ USER_DEFINED_IVT_OFFSET, 0x40

.extern __heap_start__
.extern __heap_end__

.thumb_func
.global _exit
.align 4
_exit:
        bkpt    #0
        bx      lr


.thumb_func
.global _write
.align 4
_write:
        push    {lr}
        mrs     r3, IPSR
        tst     r3, #0xff               @ 0 = thread
        bne     .system_write

        mrs     r3, CONTROL
        tst     r3, #1                  @ 1 = unprivileged
        beq     .system_write

        movs    r7, #4                  @ syscall write
        svc     #0
        pop     {pc}

        .system_write:
                bl      sys_write
                pop    {pc}

.thumb_func
.global _read
.align 4
_read:
        push    {lr}
        mrs     r3, CONTROL
        tst     r3, #1                  @ 1 - unprivileged
        beq     .system_read

        movs    r7, #3                  @ syscall read
        svc     #0
        pop     {pc}

        .system_read:
                bl      ksys_read       @ spinlocks
                pop    {pc}

/**
 * Syscall
 *  r7          - syscall number
 *  r0-r3       - arguments specified by syscalls
 */
.thumb_func
.global isr_svcall
.align 4
isr_svcall:
        push    {fp}
        mov     fp, sp

        push    {r4}
        mov     r4, lr                          @ EXC_RETURN

        cmp     r7, READ_SVC
        beq     .read_svc

        cmp     r7, WRITE_SVC
        beq     .write_svc

        cmp     r7, OS_INIT_SVC
        beq     .os_init_svc

        .read_svc:
                bl      sys_read
                b       .svcall_exit

        .write_svc:
                bl      sys_write
                b       .svcall_exit

        .os_init_svc:
                bl      scheduler_get_process_stack     @ gets a pointer to stack pointer
                ldr     r1, [r0]
                ldmia   r1!, {r2-r11}

                mov     lr, r2
                msr     CONTROL, r3
                isb

                msr     psp, r1
                str     r1, [r0]


                @ set PENDSV to the lowest priority level
                ldr     r2, NVIC_IPR_PEND_SV
                ldr     r1, PPB_BASE
                movs    r0, #0xff
                str     r0, [r1, r2]

                @ enable systick
                mov     r4, lr
                ldr     r0, =625000
                bl      systick_enable
                mov     lr, r4

                pop     {r4}
                bx      lr                              @ start proc0

        .svcall_exit:
                mov     r1, r4                          @ restore EXC_RETURN
                pop     {r4}                            @ restore original r4
                mov     sp, fp                          @ restore MSP
                pop     {fp}                            @ restore original fp

                mrs     r2, psp
                str     r0, [r2]                        @ save return value on the stack
                                                        @ upon returning from this exception
                                                        @ the stack frame is restored, at which
                                                        @ r0 is at offset 0

                bx      r1



/**
 *
 */
.thumb_func
.global isr_pendsv
.align 4
isr_pendsv:
        mrs     r0, psp

        mov     r2, lr                  @ EXC_RETURN
        mrs     r3, CONTROL
        stmdb   r0!, {r2-r11}

        msr     psp, r0                 @ save as a new stack pointer

        bl      update_process_and_get_next

        ldmia   r0!, {r2-r11}

        mov     lr, r2
        msr     CONTROL, r3
        isb                             @ executes an Instruction Synchronization
                                        @  Barrier after CONTROL updates
        msr     psp, r0

        bx      lr



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


/**
 *
 */
.thumb_func
.global isr_systick
.align 4
isr_systick:
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
NVIC_IPR_PEND_SV:       .word 0xe438        @ 14
