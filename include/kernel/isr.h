//
// Created by Artur Twardzik on 07/04/2026.
//

#ifndef OS_ISR_H
#define OS_ISR_H

#include <types.h>

#ifdef ARCH_RP2040
#define IO_IRQ_BANK0 13
#elifdef ARCH_RP2350
#define IO_IRQ_BANK0 21
#endif

void set_isr(uint32_t interrupt_number, void (*interrupt_handler)(void));

#endif //OS_ISR_H
