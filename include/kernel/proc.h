//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef PROC_H
#define PROC_H

#include "memory.h"
#include "syscall_codes.h"
#include "types.h"
#include "fs/file.h"

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

        bool kernel_mode;
        void *kstack;
};

typedef struct SpawnFileActions spawn_file_actions_t;
typedef struct SpawnAttr spawnattr_t;


void scheduler_init(void *current_main_kernel_stack);

struct Process *scheduler_get_current_process(void);

pid_t create_process_init(void (*process_entry_ptr)(void));

void context_switch_from_kernel(void);

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

int sys_execve(const char *path, char *const argv[], char *const envp[]);

int sys_exit(int status);

void kill(pid_t pid);

void yield(void);

void run_process_init(void);

#endif // PROC_H
