#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb

.syntax unified

/**
 * Performs atomic set on specified bits.
 *  r0 - hardware base location
 *  r1 - bitmask
 */
.thumb_func
.global atomic_set
.align 4
atomic_set:
        ldr     r2, ATOMIC_BITMASK_SET
        adds    r0, r0, r2
        str     r1, [r0]

        bx      lr

/**
 * Performs atomic clear on specified bits.
 *  r0 - hardware base location
 *  r1 - bitmask
 */
.thumb_func
.global atomic_clr
.align 4
atomic_clr:
        ldr     r2, ATOMIC_BITMASK_CLR
        adds    r0, r0, r2
        str     r1, [r0]

        bx      lr

/**
 * Performs atomic xor on specified bits.
 *  r0 - hardware base location
 *  r1 - bitmask
 */
.thumb_func
.global atomic_xor
.align 4
atomic_xor:
        ldr     r2, ATOMIC_BITMASK_XOR
        adds    r0, r0, r2
        str     r1, [r0]

        bx      lr


.align 4
ATOMIC_BITMASK_XOR:     .word 0x1000
ATOMIC_BITMASK_SET:     .word 0x2000
ATOMIC_BITMASK_CLR:     .word 0x3000
