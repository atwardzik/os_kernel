.cpu cortex-m0
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


/**
 *
 */
.thumb_func
.global isr_svcall
.align 4
isr_svcall:
        cmp  r7, YIELD_SVC
        bx   lr



/**
 *
 */
.thumb_func
.global isr_pendsv
.align 4
isr_pendsv:

        bx   lr

/**
 *
 */
.thumb_func
.global isr_systick
.align 4
isr_systick:

        bx   lr
