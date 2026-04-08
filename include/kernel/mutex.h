//
// Created by Artur Twardzik on 07/04/2026.
//

#ifndef OS_MUTEX_H
#define OS_MUTEX_H

#include "proc.h"

typedef struct {
        bool lock;
} mutex_t;

static inline void mutex_lock(mutex_t *mutex) {
        while (mutex->lock) {
                struct Process *process = scheduler_get_current_process();
                process->pstate = READY; //or mutex waiting queue?
                save_kernelmode_and_context_switch();
        }

        __asm__("cpsid i");

        mutex->lock = true;

        __asm__("cpsie i");
}

static inline void mutex_unlock(mutex_t *mutex) {
        __asm__("cpsid i");

        mutex->lock = false;

        __asm__("cpsie i");
}

#endif //OS_MUTEX_H
