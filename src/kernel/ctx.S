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

.macro  save_kernel_state
        mrs     r0, msp

        mov     r2, lr                  @ EXC_RETURN
        mrs     r3, CONTROL
        stmdb   r0!, {r2-r11}

        msr     msp, r0                 @ save as a new stack pointer
                                        @ returns r0 - psp after all updates
.endm

.macro  recall_state
        ldmia   r0!, {r2-r11}

        mov     lr, r2
        msr     CONTROL, r3
        isb                             @ executes an Instruction Synchronization
                                        @  Barrier after CONTROL updates
        msr     psp, r0

        bx      lr

.endm

.macro  recall_kernel_state
        ldmia   r0!, {r2-r11}

        mov     lr, r2
        msr     CONTROL, r3
        isb                             @ executes an Instruction Synchronization
                                        @  Barrier after CONTROL updates
        msr     msp, r0

        bx      lr

.endm


/**
 * Initializes stack structure, cf. save_state and recall state.
 * It has to be done in a separate function, as a new process
 * does not have any info.
 *
 * Additionally we could pass parameters to function in that manner TODO: check
 *
 *
 * void init_process_stack_frame(void **initial_sp, uint32_t xpsr, void *return_address, void *exc_return);
 */
.thumb_func
.global init_process_stack_frame
.align 4
init_process_stack_frame:
        mov     r12, r0
        ldr     r0, [r0]
        subs    r0, r0, #(18 * 4)       @ total number of registers to be stored

        str     r3, [r0]                @ save exc_return

        movs    r3, #3                  @ unprivileged task, psp, in CONTROL
        str     r3, [r0, #4]

        ldr     r3, =exit
        str     r3, [r0, #60]           @ where does the process return - default to EXIT

        str     r2, [r0, #64]           @ return to the process execution

        str     r1, [r0, #68]           @ xPSR

        str     r0, [r12]

        bx      lr

.thumb_func
.global context_switch_from_kernel
.align 4
context_switch_from_kernel:
        save_kernel_state
        mrs     r0, psp
        mrs     r1, msp

        bl      scheduler_update_process_and_get_next

        mov     r4, r0
        bl      is_in_kernel_mode
        cmp     r0, #0
        beq     .back_to_userspace
        .back_to_kernelspace:
                mov     r0, r4
                recall_kernel_state

        .back_to_userspace:
                mov     r0, r4
                recall_state

.thumb_func
.global force_context_switch
.align 4
force_context_switch:
        bl      scheduler_get_next_process

        mov     r4, r0
        bl      is_in_kernel_mode
        cmp     r0, #0
        beq     .force_back_to_userspace
        .force_back_to_kernelspace:
                mov     r0, r4
                recall_kernel_state

        .force_back_to_userspace:
                mov     r0, r4
                recall_state