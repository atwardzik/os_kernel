.cpu cortex-m33
.thumb

.syntax unified

.section .text.startup

.global _start
.align 4
.thumb_func
_start:
        @ clear bss
        ldr     r1, =main
        blx     r1

        bkpt    #0
        @ call exit (it is done automatically)
