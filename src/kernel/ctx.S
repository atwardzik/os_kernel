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

.macro  save_state
        mrs     r0, psp

        mov     r2, lr                  @ EXC_RETURN
        mrs     r3, CONTROL
        stmdb   r0!, {r2-r11}

        msr     psp, r0                 @ save as a new stack pointer
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


/**
 * void *block_on_resource(const pid_t parent_process, const Resource resource)
 */
.thumb_func
.global block_on_resource
.align 4
block_on_resource:
        push    {lr}
        push    {r0}
        bl      set_process_wait_for_resource
        pop     {r3}                                    @ temporarily PID
        @ Save kernel frame on psp
        mrs     r0, psp
        mov     r1, fp
        .copy_stack_frame:
                ldr     r2, [r1, #-4]!                  @ copy from MSP to PSP
                str     r2, [r0, #-4]!                  @ stack is full descending
                cmp     r1, sp                          @
                bne     .copy_stack_frame               @

        mov     sp, fp                                  @ clear MSP -

        @ Save full current kernel state, EXC_RETURN is not needed, as it is stored on the stack already
        @ and control register is not needed as well, as we are already in handler mode
        @ r0-r3  are not needed either, as we expect return value there
        @
        @  +------+----------------+----+----------+--------+
        @  | xPSR | Return Address | lr | r12 (ip) | r11-r4 |        => 12
        @  +------+----------------+----+----------+--------+
        @        0x2c            0x28  0x24       0x20      0x00
        @
        subs    r0, r0, #(12 * 4)                       @  total number of registers to be stored
        stmdb   r0, {r4-r11}                            @  save higher registers
        mov     r1, lr                                  @  where does the process return - CURRENT LR
        str     r1, [r0, #0x24]                         @
        ldr     r1, =.syscall_block_return_point        @  return to THIS FUNCTION
        str     r1, [r0, #0x28]                         @
        mrs     r1, PSR
        str     r1, [r0, #0x2c]
        @ Update PSP in the struct
        msr     psp, r0
        mov     r0, r3                                  @ get back PID
        bl      scheduler_get_process_stack
        mrs     r1, psp
        str     r1, [r0]
        @ Force context switch
        bl      get_next_process
        recall_state

        .syscall_block_return_point:
                @ as we return here the pointer to the resource is already in r0
                pop    {pc}


.thumb_func
.global signal_resource
.align 4
signal_resource:
        mrs     r3, CONTROL
        tst     r3, #1                  @ 1 = unprivileged
        beq     .kernel_mode_return

        @ save current process
        push    {r0, r1}

        save_state

        bl      update_process

        @ restore context of a kernel that was when the waiting process gave up control
        ldr     r2, =resource_ptr
        str     r0, [r2]

        mov     r0, r1                                  @ resource code
        bl      get_process_waiting_for_resource
        mov     r4, r0                                  @ save pid
        movs    r1, #1                                  @ pstate = RUNNING
        bl      change_process_state
        mov     r0, r4                                  @ restore pid
        bl      scheduler_get_process_stack

        @ with psp in r0, we can restore kernel stack (the opposite of block_on_resource)
        ldmia   r0, {r4-r11}                            @ restore higher registers (r11 is fp)
        ldr     r1, [r0, #0x24]
        mov     lr, r1                                  @ restore LR

        ldr     r2, =saved_pc
        ldr     r1, [r0, #0x28]                         @ get return point of the kernel mode
        str     r1, [r2]

        ldr     r1, [r0, #0x2c]
        msr     PSR, r1

        adds    r0, r0, #(12 * 4)
        mrs     r1, msp
        .restore_stack_frame:
                ldr     r2, [r0, #4]!
                str     r2, [r1, #4]!                   @ copy from PSP to MSP
                cmp                                     @ TODO: how to get how many words were saved on the stack?


        ldr     r0, =resource_ptr
        ldr     r0, [r0]
        ldr     r1, =saved_pc
        ldr     pc, [r1]
        bkpt    #0

        .kernel_mode_return:
                bx      lr

.section .data
resource_ptr:   .word 0
saved_pc:       .word 0
