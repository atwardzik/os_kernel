//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef PROC_H
#define PROC_H

#include "regs.h"
#include "memory.h"
#include <stddef.h>

enum State {
        running,
        ready,
        blocked,
};

typedef int pid_t;

struct Process {
        void *pstack;
        pid_t pid;
        enum State pstate;
        size_t allocated_memory;
};

void scheduler_init(void);

void svcall(void);

pid_t create_process(void (*process_start_ptr)(void));

pid_t fork(void);

void exit(int exit_code);

void kill(pid_t pid);

void yield(void);

#endif //PROC_H
