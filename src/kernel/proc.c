//
// Created by Artur Twardzik on 30/12/2024.
//

#include "proc.h"

#include "memory.h"
#include "tty.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

static constexpr size_t INITIAL_PROCESS_COUNT = 20;

static constexpr int DEFAULT_PROCESS_SIZE = 8 * 1024; // 8 [KB]

static constexpr off_t DEFAULT_PROCESS_SP_OFFSET = 6 * 1024;

#define THREAD_PSP_CODE ((void *) 0xfffffffd)

#define RECALL_STATE_FROM(sp)                           \
        __asm__(                                        \
                "mrs    r0," STR(sp) "\n\r"             \
                "ldmia  r0!, {r2-r11}\n\r"             \
                "mov    lr, r2\n\r"                    \
                "msr    CONTROL, r3\n\r"               \
                "isb\n\r"                               \
                "msr    " STR(sp) ", r0\n\r"           \
                "bx     lr\n\r"                        \
        );

#define RECALL_USERSPACE_STATE RECALL_STATE_FROM(psp)
#define RECALL_KERNELSPACE_STATE RECALL_STATE_FROM(msp)

static struct {
        size_t max_processes;
        size_t processes_count;
        struct Process *processes;
        struct Process *current_process;

        size_t total_allocated_memory;

        void *main_kernel_stack; // TODO: see if it is needed after processes start running
} scheduler __attribute__((section(".data")));

extern void init_process_stack_frame(void **initial_sp, uint32_t xpsr, void *pc, void *lr);

void scheduler_init(void *current_main_kernel_stack) {
        scheduler.max_processes = INITIAL_PROCESS_COUNT;
        scheduler.processes_count = 0;

        struct Process *processes = kmalloc(sizeof(struct Process) * INITIAL_PROCESS_COUNT);
        scheduler.processes = processes;
        for (size_t i = 0; i < INITIAL_PROCESS_COUNT; ++i) {
                processes[i].ptr = nullptr;
        }

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


static struct Process *scheduler_get_next_process() {
        static size_t current_index = 0;

        if (scheduler.processes_count == 0) {
                return nullptr;
        }
        else if (scheduler.processes_count == 1) {
                return scheduler.current_process;
        }

        do {
                // TODO: determine task importance, also by implementing priority queue. FIXME: this may spinlock
                current_index += 1;
                if (current_index == scheduler.max_processes) {
                        current_index = 0;
                }
        } while (scheduler.processes[current_index].pstate != READY);

        return &scheduler.processes[current_index];
}

void scheduler_update_process(void *psp, void *msp) {
        if (!scheduler.current_process->kernel_mode) {
                scheduler.current_process->pstate = READY;
        }

        scheduler.current_process->pstack = psp;
        scheduler.current_process->kstack = msp;
}

bool is_in_kernel_mode() { return scheduler.current_process->kernel_mode; }

void set_kernel_mode_flag() { scheduler.current_process->kernel_mode = true; }

void reset_kernel_mode_flag() { scheduler.current_process->kernel_mode = false; }

void context_switch(void) {
        struct Process *process = scheduler_get_next_process();

        scheduler.current_process = process;
        process->pstate = RUNNING;


        __asm__("msr    psp, %0\n\r" : : "r"(process->pstack));
        __asm__("msr    msp, %0\n\r" : : "r"(process->kstack));

        if (process->kernel_mode) {
                RECALL_KERNELSPACE_STATE
        }
        else {
                RECALL_USERSPACE_STATE
        }
}

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
        scheduler.processes_count += 1;
        current->children[current->children_count] = &scheduler.processes[pid];

        pid += 1;
        return process.pid;
}


pid_t create_process_init(void (*process_entry_ptr)(void)) {
        if (scheduler.current_process || scheduler.processes[0].ptr) {
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
        scheduler.processes_count = 1;
        scheduler.total_allocated_memory += DEFAULT_PROCESS_SIZE;

        return process.pid;
}


void sys_exit(int status) {
        struct Process *current = scheduler.current_process;

        current->exit_code = status;
        current->exit_signal = SIGCHLD;

        if (current->parent->wait_child_exit) {
                current->parent->pstate = READY;
                current->parent->wait_child_exit = false;
        }

        for (size_t i = 0; i < current->children_count; ++i) {
                signal_notify(current->children[i], SIGHUP);
        }

        __asm__("msr    msp, %0\n\r" : : "r"(scheduler.main_kernel_stack));
        context_switch();
}

void sys_kill(const pid_t pid) {
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
        kfree(current->ptr);

        const struct Process p = {};
        scheduler.processes[pid] = p;
}

void run_process_init(void) {
        scheduler.current_process = &scheduler.processes[0];
        scheduler.current_process->pstate = RUNNING;
        __asm__("movs   r0, #0\n\r"  // run init (pid=0)
                "svc    #0xff\n\r"); // OS_INIT_SVC
}

void signal_notify(struct Process *process, const int sig) {
        if (process->sighandlers[sig]) {
                (*process->sighandlers[sig])();
        }
}
