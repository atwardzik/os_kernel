#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb

.syntax unified


/**
 *
 */
.thumb_func
.global init_process_stack
.align 4
init_process_stack:
        mov  r0, r0
        bx   lr



/**
 *
 */
.thumb_func
.global save_state
.align 4
save_state:

        bx   lr
