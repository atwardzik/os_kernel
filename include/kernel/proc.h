//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef PROC_H
#define PROC_H

#include "fs/file.h"
#include "memory.h"
#include "syscalls.h"

#include <stddef.h>
#include <sys/types.h>

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
        NEW                  = 0,
        RUNNING              = 1,
        READY                = 2,
        WAITING_FOR_RESOURCE = 3,
        TERMINATED           = 4,
};

// typedef unsigned int pid_t;

// TODO: by using MPU forbid process to access system resources
struct Process {
        void *ptr;
        void *pstack;
        pid_t pid;
        enum State pstate;
        size_t allocated_memory;
        unsigned int priority_level;
        struct Files files;

        bool kernel_mode;
        void *kstack;
};


void scheduler_init(void *current_main_kernel_stack);

struct Process *scheduler_get_current_process(void);

struct Process *create_init_process();

pid_t create_process(void (*process_entry_ptr)(void));

void change_process_state(pid_t process, enum State state);

void context_switch_from_kernel(void);

pid_t fork(void);

int sys_exit(int status);

void kill(pid_t pid);

void yield(void);

[[deprecated("Debug only, use fork instead.")]]
void run_process_init(void);

#endif // PROC_H
