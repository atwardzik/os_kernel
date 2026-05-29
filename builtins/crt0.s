.cpu cortex-m33
.thumb

.syntax unified

.section .text.startup

.global _start
.align 4
.thumb_func
_start:
        @ clear bss

        bl      main    @ must be done with bl to be position independent

        bkpt    #0
        @ call exit (it is done automatically)
