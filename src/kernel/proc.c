//
// Created by Artur Twardzik on 30/12/2024.
//

#include "proc.h"

#include "tty.h"


static constexpr size_t INITIAL_PROCESS_COUNT = 20;

static constexpr int DEFAULT_PROCESS_SIZE = 8 * 1024; // 8 [KB]

static constexpr off_t DEFAULT_PROCESS_SP_OFFSET = 6 * 1024;

#define THREAD_PSP_CODE ((void *) 0xfffffffd)

static struct {
        size_t max_processes;
        struct Process *processes;
        struct Process *current_process;

        size_t total_allocated_memory;

        void *main_kernel_stack; // TODO: see if it is needed after processes start running
} scheduler __attribute__((section(".data")));

extern void init_process_stack_frame(void **initial_sp, uint32_t xpsr, void *pc, void *lr);

void scheduler_init(void *current_main_kernel_stack) {
        scheduler.max_processes = INITIAL_PROCESS_COUNT;

        struct Process *processes = kmalloc(sizeof(struct Process) * INITIAL_PROCESS_COUNT);
        for (size_t i = 0; i < INITIAL_PROCESS_COUNT; ++i) {
                scheduler.processes[i].ptr = nullptr;
        }
        scheduler.processes = processes;

        scheduler.total_allocated_memory = 0;
        scheduler.current_process = nullptr;

        scheduler.main_kernel_stack = current_main_kernel_stack;
}

struct Process *scheduler_get_current_process() { return scheduler.current_process; }

void **scheduler_get_kernel_stack(void) {
        if (!scheduler.current_process) {
                return nullptr;
        }

        return &scheduler.current_process->kstack;
}

void **scheduler_get_process_stack(const pid_t current_process) {
        size_t index = 0;
        while (scheduler.processes[index].pid != current_process) {
                index += 1;

                if (index == scheduler.max_processes) {
                        return nullptr;
                }
        }

        return &scheduler.processes[index].pstack;
}


void *scheduler_get_next_process() {
        static size_t current_index = 0;

        do {
                // TODO: determine task importance, also by implementing priority queue
                current_index += 1;
                if (current_index == scheduler.max_processes) {
                        current_index = 0;
                }
        } while (scheduler.processes[current_index].pstate != READY);

        scheduler.current_process = &scheduler.processes[current_index];

        scheduler.current_process->pstate = RUNNING;


        if (scheduler.current_process->kernel_mode) {
                __asm__("msr    psp, %0\n\r" : : "r"(scheduler.current_process->pstack));
                return scheduler.current_process->kstack;
        }
        else {
                __asm__("msr    msp, %0\n\r" : : "r"(scheduler.current_process->kstack));
                return scheduler.current_process->pstack;
        }
}

static void scheduler_update_process(void *psp, void *msp) {
        if (!scheduler.current_process->kernel_mode) {
                scheduler.current_process->pstate = READY;
        }

        scheduler.current_process->pstack = psp;
        scheduler.current_process->kstack = msp;
}

void *scheduler_update_process_and_get_next(void *psp, void *msp) {
        scheduler_update_process(psp, msp);

        return scheduler_get_next_process();
}


bool is_in_kernel_mode() { return scheduler.current_process->kernel_mode; }

void set_kernel_mode_flag() { scheduler.current_process->kernel_mode = true; }

void reset_kernel_mode_flag() { scheduler.current_process->kernel_mode = false; }

pid_t sys_spawn_process(
        void (*process_entry_ptr)(void),
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
) {
        static pid_t pid = 1;

        if (pid >= scheduler.max_processes) {
                __asm__("bkpt   #0");
        }

        struct Process *current = scheduler.current_process;

        void *process_page = kmalloc(DEFAULT_PROCESS_SIZE);
        void *kstack = process_page + DEFAULT_PROCESS_SIZE - sizeof(size_t);
        void *pstack = process_page + DEFAULT_PROCESS_SP_OFFSET - sizeof(size_t);
        init_process_stack_frame(&pstack, 0x0100'0000, process_entry_ptr, THREAD_PSP_CODE);


        struct File **fdtable = kmalloc(sizeof(struct File *) * current->files.count);
        struct Files files = {current->files.count, fdtable};

        for (size_t i = 0; i < current->files.count; ++i) {
                files.fdtable[i] = current->files.fdtable[i];
        }

        struct Process process = {
                .ptr = process_page,
                .pstack = pstack,
                .pid = pid,
                .pstate = READY,
                .allocated_memory = current->allocated_memory,
                .priority_level = current->priority_level,
                .files = files,
                .parent = current,
                .max_children_count = current->max_children_count - 1,
                .children_count = 0,
                .children = kmalloc(sizeof(struct Process *) * (current->max_children_count - 1)),
                .kernel_mode = false,
                .kstack = kstack
        };
        for (size_t i = 0; i < process.max_children_count; ++i) {
                process.children[i] = nullptr;
        }

        scheduler.total_allocated_memory += process.allocated_memory;
        scheduler.processes[pid] = process;
        current->children[current->children_count] = &scheduler.processes[pid];

        pid += 1;
        return process.pid;
}


pid_t create_process_init(void (*process_entry_ptr)(void)) {
        if (scheduler.current_process) {
                __asm__("bkpt   #0");
        }

        void *process_page = kmalloc(DEFAULT_PROCESS_SIZE);
        void *kstack = process_page + DEFAULT_PROCESS_SIZE - sizeof(size_t);
        void *pstack = process_page + DEFAULT_PROCESS_SP_OFFSET - sizeof(size_t);
        init_process_stack_frame(&pstack, 0x0100'0000, process_entry_ptr, THREAD_PSP_CODE);

        struct Process process = {
                .ptr = process_page,
                .pstack = pstack,
                .pid = 0,
                .pstate = READY,
                .allocated_memory = DEFAULT_PROCESS_SIZE,
                .priority_level = 0,
                .files = create_tty_file_mock(),
                .parent = nullptr,
                .max_children_count = INITIAL_PROCESS_COUNT - 1,
                .children_count = 0,
                .children = kmalloc(sizeof(struct Process *) * (INITIAL_PROCESS_COUNT - 1)),
                .kernel_mode = false,
                .kstack = kstack
        };
        for (size_t i = 0; i < process.max_children_count; ++i) {
                process.children[i] = nullptr;
        }

        scheduler.processes[0] = process;
        scheduler.total_allocated_memory += DEFAULT_PROCESS_SIZE;

        return process.pid;
}

extern void force_context_switch(void);

int sys_exit(int status) { //and kill all children
        struct Process *current = scheduler.current_process;

        for (size_t i = 0; i < current->children_count; ++i) {
                kill(current->children[i]->pid);
        }

        kill(current->pid);

        __asm__("msr    msp, %0\n\r" : : "r"(scheduler.main_kernel_stack));
        force_context_switch();
}

void kill(const pid_t pid) {
        struct Process *current = &scheduler.processes[pid];
        if (!current->ptr) {
                return;
        }

        for (size_t i = 0; i < current->files.count; ++i) {
                struct File *file = current->files.fdtable[i];

                if (file->f_owner == current->pid) {
                        kfree(file->f_op);
                        kfree(file);
                }
        }

        kfree(current->files.fdtable);
        kfree(current);

        const struct Process p = {};
        scheduler.processes[pid] = p;
}

void run_process_init(void) {
        scheduler.current_process = &scheduler.processes[0];
        scheduler.current_process->pstate = RUNNING;
        __asm__("movs   r0, #0\n\r"  // run init (pid=0)
                "svc    #0xff\n\r"); // OS_INIT_SVC
}
