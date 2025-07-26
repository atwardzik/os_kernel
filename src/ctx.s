.cpu cortex-m0
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
