#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
.fpu fpv5-sp-d16
#endif
.thumb

.syntax unified


/**
 * Initializes stack structure, cf. save_state and recall state.
 * It has to be done in a separate function, as a new process
 * does not have any info.
 *
 * Additionally we could pass parameters to function in that manner TODO: check
 *
 *
 * void init_process_stack_frame(void **initial_sp, uint32_t xpsr, uint32_t return_address, uint32_t exc_return);
 */
.thumb_func
.global init_process_stack_frame
.align 4
init_process_stack_frame:
        @ mov     r12, r0
        @ ldr     r0, [r0]
        @ subs    r0, r0, 51 * 4          @ total number of registers to be stored

        @ str     r3, [r0]                @ save exc_return

        @ movs    r3, #3                  @ unprivileged task in CONTROL_NS
        @ str     r3, [r0, #4]

        @ ldr     r3, =exit
        @ str     r3, [r0, #0x78]         @ where does the process return - default to EXIT

        @ str     r2, [r0, #0x7c]         @ return to the process execution

        @ str     r1, [r0, #0x80]         @ xPSR

        @ str     r0, [r12]

        mov     r12, r0
        ldr     r0, [r0]
        subs    r0, r0, #(18 * 4)       @ total number of registers to be stored

        str     r3, [r0]                @ save exc_return

        movs    r3, #3                  @ unprivileged task, psp, in CONTROL_NS
        str     r3, [r0, #4]

        ldr     r3, =exit
        str     r3, [r0, #60]           @ where does the process return - default to EXIT

        str     r2, [r0, #64]           @ return to the process execution

        str     r1, [r0, #68]           @ xPSR

        str     r0, [r12]

        bx      lr



/**
 * This method saves all remaining registers to current process stack.
 *
 * After interrupt of the current process lower registers are stored on the stack.
 * In order to switch the context properly, other registers must be saved as well.
 *
 * (Fig. 1) Exception Stack Frame Format after entering ISR:
 *  ++--------+-------++------+----------------+----+----------+-------+
 *  || s15-s0 | FPSCR || xPSR | Return Address | lr | r12 (ip) | r3-r0 |        => 25 registers / 8
 *  ++--------+-------++------+----------------+----+----------+-------+
 *  ||                ||                                               ^
 *  || if FPU present ||      |                |    |          |       |
 *  ||                ||     0x1c             0x18 0x14       0x10     |-PSP
 *
 * Please note the return address and link register.
 * During normal function call the return address is saved into the link register,
 * however, during exception a special EXC_RETURN (different in various exception scenarios)
 * value is loaded by hardware into lr to indicate return from EXCEPTION when it is loaded
 * into the PC. *That is why the return address is loaded into stack frame.*
 *
 * (Fig. 2) Process stack after saving all registers by this method.
 *  +----------++---------++--------+------------+------------+
 *  | (Fig. 1) || s16-s31 || r11-r4 | CONTROL_NS | EXC_RETURN |                 => 25 + 26 = 51 registers / 10
 *  +----------++---------++--------+------------+------------+
 *             ||         ||                                  ^
 *             ||         ||        |            |            |
 *            0x64        ||       0x08         0x04          |-PSP
 */
.thumb_func
.global save_state
.align 4
save_state:
        mrs     r0, psp

#ifdef ARCH_RP2350
        @ tst     lr, #0x10               @ Tests bit 4 of EXC_RETURN if FPU exists
        @ it      eq
        @ vstmdbeq r0!, {s16-s31}
#endif
        mov     r2, lr                  @ EXC_RETURN
        mrs     r3, CONTROL
        stmdb   r0!, {r2-r11}

        msr     psp, r0                 @ save as a new stack pointer
                                        @ returns r0 - psp after all updates
        bx      lr


.thumb_func
.global recall_state
.align 4
recall_state:
        ldmia   r0!, {r2-r11}

        mov     lr, r2
        msr     CONTROL, r3
        isb                             @ executes an Instruction Synchronization
                                        @  Barrier after CONTROL updates
#ifdef ARCH_RP2350
        @ tst     lr, #0x10
        @ it      eq
        @ vldmiaeq r0!, {s16-s31}
#endif
        msr     psp, r0

        bx      lr
