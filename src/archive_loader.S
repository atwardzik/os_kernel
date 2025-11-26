#ifdef ARCH_RP2040
.cpu cortex-m0
#elifdef ARCH_RP2350
.cpu cortex-m33
#endif
.thumb

.syntax unified

@.section .text

.section .cpio_init, "wa"
.incbin "builtins/initramfs.cpio"