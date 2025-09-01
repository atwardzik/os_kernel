//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef PROC_H
#define PROC_H

#include "memory.h"
#include "syscalls.h"

#include <stddef.h>

/*
 * The context switch routine has to:
 * - Manually stack remaining registers r4-r11 on the Process Stack
 * - Save current task’s PSP to memory
 * - Load next task’s stack pointer and assign it to PSP
 * - Manually unstack registers r4-r11
 * - Call bx 0xfffffffD which makes the processor switch to Unprivileged Handler Mode,
 *   unstack next task’s exception frame and continue on its PC.
 */

enum State {
        NEW,
        RUNNING,
        READY,
        WAITING_FOR_RESOURCE,
        SUSPENDED,
        TERMINATED,
};

typedef unsigned int pid_t;

//TODO: by using MPU forbid process to access system resources
struct Process {
        void *ptr;
        void *pstack;
        pid_t pid;
        enum State pstate;
        unsigned int priority_level;
        size_t allocated_memory;
};

void scheduler_init(void);

pid_t create_process(void (*process_entry_ptr)(void));

pid_t fork(void);

int exit(void);

void kill(pid_t pid);

void yield(void);

[[deprecated("Debug only, use fork instead.")]]
void run_all_processes(void);

#endif //PROC_H
