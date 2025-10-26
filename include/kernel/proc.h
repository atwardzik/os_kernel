//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef PROC_H
#define PROC_H

#include "resources.h"
#include "types.h"
#include "fs/file.h"

#include <sys/signal.h>

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

// TODO: by using MPU forbid process to access system resources
struct Process {
        void *ptr;
        void *pstack;
        pid_t pid;
        enum State pstate;
        size_t allocated_memory;
        unsigned int priority_level;
        struct Files files;

        struct Process *parent;
        size_t max_children_count;
        size_t children_count;
        struct Process **children;
        bool wait_child_exit;

        int exit_code;
        int exit_signal; // the parent will receive this signal and clean up after the child

        void (*sighandlers[32])(void);

        bool kernel_mode;
        void *kstack;
};

typedef struct SpawnFileActions spawn_file_actions_t;
typedef struct SpawnAttr spawnattr_t;


void scheduler_init(void *current_main_kernel_stack);

struct Process *scheduler_get_current_process(void);

void scheduler_update_process(void *psp, void *msp);

/**
 * Performs context switch without saving the current process. Scheduler
 * chooses a new process and switches to it.
 */
[[noreturn]] void context_switch(void);

/**
 * Performs standard context switch, i.e. the current process was in user mode.
 *
 * The context of the current process is saved on it's PSP, scheduler chooses
 * a new process and switches to it.
 */
[[noreturn]] void save_usermode_and_context_switch(void);

/**
 * Similar to standard context switch, but the current process was in kernel mode.
 *
 * The context of the current process is saved on it's MSP, scheduler chooses
 * a new process and switches to it.
 */
[[noreturn]] void save_kernelmode_and_context_switch(void);


pid_t create_process_init(void (*process_entry_ptr)(void));

/**
 * Creates a new process, based on the current process.
 * @param process_entry_ptr execution start pointer
 * @param file_actions settings for copying fdtable
 * @param attrp priority and permissions for spawned process
 * @param argv arguments passed
 * @param envp is an array of pointers to strings, conventionally of the form key=value,
 *             which are passed as the environment of the new program
 * @return positive for successful child creation, negative for error
 */
pid_t sys_spawn_process(
        void (*process_entry_ptr)(void),
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
);

// int sys_execve(const char *path, char *const argv[], char *const envp[]);

/**
 * Suspends execution of current process until stat_loc information is available for a terminated child process,
 * or a signal is received (e.g. SIGCHLD, SIGTERM).
 *
 * @param stat_loc contains bitwise OR of options. SIGSTOP/SIGCHLD | exit_code
 * @return the process ID of the child to the calling process. Otherwise, a value of -1.
 */
pid_t sys_wait(int *stat_loc);

/**
 * Sets current state to TERMINATED and performs context switch
 *
 * @param status is an exit code that will be passed to the parent
 */
[[noreturn]] void sys_exit(int status);

void sys_kill(pid_t pid);

void yield(void);

void run_process_init(void);

void signal_notify(struct Process *process, int sig);

#endif // PROC_H
