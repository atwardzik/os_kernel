.cpu cortex-m33
.thumb

.syntax unified

.section .text.startup

.global _start
.align 4
.thumb_func
_start:
        @ clear bss
        mov     r7, #0
        push    {r7, lr}

        bl      main            @ must be done with bl to be position independent

        pop     {r7, pc}        @ call exit (it is done automatically)
