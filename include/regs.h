//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef REGS_H
#define REGS_H

#include <stdint.h>

struct registers {
        uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, cpsr;
};

#define fp r11
#define ip r12
#define sp r13
#define lr r14
#define pc r15

#endif //REGS_H
