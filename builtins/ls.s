.cpu cortex-m33
.thumb
.syntax unified

.thumb_func
.global main
.align 4
main:
        push    {r7, lr}
        sub     sp, sp, #56
        add     r7, sp, #0
        str     r0, [r7, #4]    @ str argc
        str     r1, [r7]        @ str argv (char **argv)
        ldr     r3, [r7]        @ load argv ptr
        ldr     r3, [r3, #4]    @ load argv[1]

        ldr     r1, =0x200000   @ O_DIRECTORY | O_RDONLY
        mov     r0, r3
        svc     #5              @ syscall open
        str     r0, [r7, #52]   @ store dirfd

        b       .L2
.L3:
        add     r3, r7, #12
        adds    r3, r3, #8

        push    {r3}
        mov     r0, r3
        bl      strlen
        mov     r2, r0
        mov     r0, #1          @ stdout
        pop     {r1}            @ names
        svc     #4              @ write svc

        mov     r0, #1
        adr     r1, endl
        mov     r2, #1
        svc     #4
.L2:
        add     r3, r7, #12
        mov     r1, r3
        ldr     r0, [r7, #52]
        svc     #8              @ syscall readdir
        mov     r3, r0
        cmp     r3, #0
        bne     .L3
        movs    r3, #0
        mov     r0, r3
        adds    r7, r7, #56
        mov     sp, r7

        mov     r0, #0
        pop     {r7, lr}
        svc     #1

.thumb_func
.align 4
strlen:
        movs    r1, #0
        .loop:
                ldr     r2, [r0]
                cmp     r2, #0
                beq     .end_loop
                adds    r1, r1, #1
                adds    r0, r0, #1
                b       .loop

        .end_loop:
                mov     r0, r1
                bx      lr


endl:   .asciz  "\n"
