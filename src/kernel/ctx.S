#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
.fpu fpv5-sp-d16
#endif
.thumb

.syntax unified

/**
 * All remaining registers must be saved to current process stack.
 * On saving the usermode state we have to save registers on process stack pointer,
 * however if we are in kernel mode, e.g. calling wait_event_interruptible() we
 * must save on the kernel stack pointer (assigned to this process!)
 *
 * After interrupt of the current process lower registers are stored on the stack.
 * In order to switch the context properly, other registers must be saved as well.
 *
 * (Fig. 1) Exception Stack Frame Format after entering ISR:
 * @code
 *  ++--------+-------++------+----------------+----+----------+-------+
 *  || s15-s0 | FPSCR || xPSR | Return Address | lr | r12 (ip) | r3-r0 |        => 25 (FPU) or 8
 *  ++--------+-------++------+----------------+----+----------+-------+
 *  ||                ||                                               ^
 *  || if FPU present ||      |                |    |          |       |
 *  ||                ||     0x1c             0x18 0x14       0x10     |-PSP
 * @endcode
 *
 * Please note the return address and link register.
 * During normal function call the return address is saved into the link register,
 * however, during exception a special EXC_RETURN (different in various exception scenarios)
 * value is loaded by hardware into lr to indicate return from EXCEPTION when it is loaded
 * into the PC. <b>That is why the return address is loaded into stack frame.</b>
 *
 * (Fig. 2) Process stack after saving all registers by this method.
 * @code
 *  +----------++---------++--------+------------+------------+
 *  | (Fig. 1) || s16-s31 || r11-r4 | CONTROL_NS | EXC_RETURN |                 => + 26 (FPU) or 10
 *  +----------++---------++--------+------------+------------+
 *             ||         ||                                  ^
 *             ||         ||        |            |            |
 *            0x64        ||       0x08         0x04          |-PSP
 * @endcode
 */

.macro  save_kernel_state
        mrs     r0, msp

        mov     r2, lr                  @ EXC_RETURN
        mrs     r3, CONTROL
        stmdb   r0!, {r2-r11}

        msr     msp, r0                 @ save as a new stack pointer
                                        @ returns r0 - psp after all updates
.endm

.macro  save_state
        mrs     r0, psp

        mov     r2, lr                  @ EXC_RETURN
        mrs     r3, CONTROL
        stmdb   r0!, {r2-r11}

        msr     psp, r0                 @ save as a new stack pointer
                                        @ returns r0 - psp after all updates
.endm

/**
 * Initializes stack structure, cf. save_state and recall state.
 * It has to be done in a separate function, as a new process
 * does not have any info.
 *
 * Additionally we could pass parameters to function in that manner TODO: check
 *
 *
 * void create_process_stack_frame(void **initial_sp, void *lr, void *pc, void *exc_return);
 */
.thumb_func
.global create_process_stack_frame
.align 4
create_process_stack_frame:
        mov     r12, r0
        ldr     r0, [r0]
        subs    r0, r0, #(18 * 4)       @ total number of registers to be stored

        str     r3, [r0]                @ save exc_return

        movs    r3, #3                  @ unprivileged task, psp, in CONTROL
        str     r3, [r0, #4]

        str     r1, [r0, #60]           @ where does the process return (lr)

        str     r2, [r0, #64]           @ return to the process execution (pc)

        ldr     r3, =0x01000000         @ initial xPSR
        str     r3, [r0, #68]           @ xPSR

        str     r0, [r12]

        bx      lr

.thumb_func
.global save_usermode_and_context_switch
.align 4
save_usermode_and_context_switch:
        save_state
        mrs     r0, psp
        mrs     r1, msp

        bl      scheduler_update_process

        bl      context_switch

.thumb_func
.global save_kernelmode
.align 4
save_kernelmode:
        save_kernel_state
        mrs     r0, psp
        mrs     r1, msp

        bl      scheduler_update_process

.thumb_func
.global save_kernelmode_and_context_switch
.align 4
save_kernelmode_and_context_switch:
        save_kernel_state
        mrs     r0, psp
        mrs     r1, msp

        bl      scheduler_update_process

        bl      context_switch
