//
// Created by Artur Twardzik on 01/04/2026.
//

#include "error.h"
#include "libc.h"

void kernel_panic(const char *msg, const char *file, int line, const char *func) {
        __asm__("cpsid i");

        printf("\n--- KERNEL PANIC ---\n");
        printf("  %s\n", msg);
        printf("  at %s:%d in %s()\n", file, line, func);

        while (1) {
                __asm__("nop");
        }
}
