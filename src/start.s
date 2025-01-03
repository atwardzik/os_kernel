.cpu cortex-m0
.thumb

.syntax unified

.section .vectors, "ax"
.align 2
.global vector_table
.thumb_func
vector_table:
.word 0x20040000        @ ? let's assume it's the end of RAM, and we'll see
.word reset             @
.word isr_nmi           @
.word isr_hardfault     @
.word isr_invalid       @ Reserved, should never fire
.word isr_invalid       @ Reserved, should never fire
.word isr_invalid       @ Reserved, should never fire
.word isr_invalid       @ Reserved, should never fire
.word isr_invalid       @ Reserved, should never fire
.word isr_invalid       @ Reserved, should never fire
.word isr_invalid       @ Reserved, should never fire
.word isr_svcall        @
.word isr_invalid       @ Reserved, should never fire
.word isr_invalid       @ Reserved, should never fire
.word isr_pendsv
.word isr_systick
irq_1_31: .fill 32, 4, 0


.macro decl_isr_bkpt name
.weak \name
.type \name,%function
.thumb_func
\name:
    bkpt #0
.endm

decl_isr_bkpt isr_nmi
decl_isr_bkpt isr_hardfault
decl_isr_bkpt isr_svcall
decl_isr_bkpt isr_invalid
decl_isr_bkpt isr_pendsv
decl_isr_bkpt isr_systick


.section .reset, "ax"

.thumb_func
.global reset
.align 4
reset:
    ldr  r2, PPB_BASE
    ldr  r1, VTOR_OFFSET
    add  r1, r1, r2
    ldr  r0, =vector_table
    str  r0, [r1]

    ldr  r1, SRAM_STRIPED_END
    mov  sp, r1

platform_entry:
    ldr  r1, =main
    blx  r1
    mov  r0, r0
    bkpt #0     @ should not return


.align 4
PPB_BASE:           .word 0xe0000000
VTOR_OFFSET:        .word 0xed08
SRAM_STRIPED_END:   .word 0x20040000
