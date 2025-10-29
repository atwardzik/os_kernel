//
// Created by Artur Twardzik on 30/12/2024.
//

#include "proc.h"

#include "memory.h"
#include "signal.h"
#include "tty.h"

#include <stdlib.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

static constexpr size_t INITIAL_PROCESS_COUNT = 20;

static constexpr int DEFAULT_PROCESS_SIZE = 8 * 1024; // 8 [KB]

static constexpr off_t DEFAULT_PROCESS_SP_OFFSET = 6 * 1024;

#define RECALL_STATE_FROM(sp)                           \
        __asm__(                                        \
                "mrs    r0," STR(sp) "\n\r"             \
                "ldmia  r0!, {r2-r11}\n\r"              \
                "mov    lr, r2\n\r"                     \
                "msr    CONTROL, r3\n\r"                \
                "isb\n\r"                               \
                "msr    " STR(sp) ", r0\n\r"            \
                "bx     lr\n\r"                         \
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


void scheduler_init(void *current_main_kernel_stack) {
        scheduler.max_processes = INITIAL_PROCESS_COUNT;
        scheduler.processes_count = 0;

        struct Process *processes = kmalloc(sizeof(struct Process) * INITIAL_PROCESS_COUNT);
        scheduler.processes = processes;
        for (size_t i = 0; i < INITIAL_PROCESS_COUNT; ++i) { //TODO: this should be DYNAMIC
                processes[i].ptr = nullptr;
        }

        scheduler.total_allocated_memory = 0;
        scheduler.current_process = nullptr;

        scheduler.main_kernel_stack = current_main_kernel_stack;
}

struct Process *scheduler_get_current_process() { return scheduler.current_process; }

void **scheduler_get_current_kernel_stack(void) {
        if (!scheduler.current_process) {
                return nullptr;
        }

        return &scheduler.current_process->kstack;
}

void **scheduler_get_current_process_stack(void) {
        if (!scheduler.current_process) {
                return nullptr;
        }

        return &scheduler.current_process->pstack;
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

__attribute__((optimize("omit-frame-pointer")))
void context_switch(void) {
        // While it might be tempting to "refactor" this code and extract the forthcoming inline assembly
        // it is not the best idea. The compiler CANNOT guarantee (unfortunately) that the current process
        // pointer will be stored in a register. Therefore, the stack pointers must be restored
        // just before context switching. Moreover, there is a hierarchy of handling:
        //      handled signals -> pending signals -> kernel mode -> user mode
        register struct Process *process = scheduler_get_next_process();
        process->pstate = RUNNING;

        scheduler.current_process = process;

        if (process->signal_handled) {
                goto switch_to_userspace;
        }

        int pending_signal = get_pending_signal();
        if (pending_signal >= 0) {
                handle_pending_signal(pending_signal);
                goto switch_to_userspace;
        }

        if (process->kernel_mode) {
                goto switch_to_kernelspace;
        }

switch_to_userspace:
        __asm__("msr    psp, %0\n\r" : : "r"(process->pstack));
        __asm__("msr    msp, %0\n\r" : : "r"(process->kstack));
        RECALL_USERSPACE_STATE

switch_to_kernelspace:
        __asm__("msr    psp, %0\n\r" : : "r"(process->pstack));
        __asm__("msr    msp, %0\n\r" : : "r"(process->kstack));
        RECALL_KERNELSPACE_STATE
}

static struct Process *create_blank_process(void (*process_entry_ptr)(void)) {
        static pid_t pid = 0;

        if (pid >= scheduler.max_processes) {
                __asm__("bkpt   #0");
        }

        void *process_page = kmalloc(DEFAULT_PROCESS_SIZE);
        void *kstack = process_page + DEFAULT_PROCESS_SIZE - sizeof(size_t);
        void *pstack = process_page + DEFAULT_PROCESS_SP_OFFSET - sizeof(size_t);
        create_process_stack_frame(&pstack,
                                   &exit,
                                   process_entry_ptr,
                                   EXC_RETURN_THREAD_PSP_CODE);

        struct Process process = {
                .ptr = process_page,
                .pstack = pstack,
                .pid = pid,
                .pstate = NEW,
                .allocated_memory = DEFAULT_PROCESS_SIZE,
                .priority_level = 0,

                .parent = nullptr,
                .max_children_count = 0,
                .children_count = 0,
                .children = nullptr,
                .wait_child_exit = false,

                .signal_mask = 0,
                .pending_signals = nullptr,
                .signal_handled = false,

                .kernel_mode = false,
                .kstack = kstack
        };
        init_default_sighandlers(&process);

        scheduler.total_allocated_memory += process.allocated_memory;
        scheduler.processes[pid] = process;
        scheduler.processes_count += 1;

        struct Process *added_process = &scheduler.processes[pid];
        pid += 1;
        return added_process;
}

pid_t sys_spawn_process(
        void (*process_entry_ptr)(void),
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
) {
        if (!scheduler.processes[0].ptr) {
                __asm__("bkpt   #0");
        }

        struct Process *current = scheduler.current_process;
        struct File **fdtable = kmalloc(sizeof(struct File *) * current->files.count);
        struct Files files = {current->files.count, fdtable};
        for (size_t i = 0; i < current->files.count; ++i) {
                files.fdtable[i] = current->files.fdtable[i];
        }

        struct Process *process = create_blank_process(process_entry_ptr);
        if (!process->ptr) {
                return -1;
        }

        process->files = files;
        process->parent = current;
        process->max_children_count = current->max_children_count - 1;
        process->children = kmalloc(sizeof(struct Process *) * (current->max_children_count - 1));
        for (size_t i = 0; i < process->max_children_count; ++i) {
                process->children[i] = nullptr;
        }

        current->children[current->children_count] = process;
        current->children_count += 1;

        process->pstate = READY;
        return process->pid;
}


pid_t create_process_init(void (*process_entry_ptr)(void)) {
        if (scheduler.current_process || scheduler.processes[0].ptr) {
                __asm__("bkpt   #0");
        }

        struct Process *process = create_blank_process(process_entry_ptr);
        if (!process->ptr) {
                __asm__("bkpt   #0");
        }

        process->files = create_tty_file_mock();
        process->max_children_count = INITIAL_PROCESS_COUNT - 1;
        process->children = kmalloc(sizeof(struct Process *) * (INITIAL_PROCESS_COUNT - 1));
        for (size_t i = 0; i < process->max_children_count; ++i) {
                process->children[i] = nullptr;
        }

        process->pstate = READY;
        return process->pid;
}


void sys_exit(int status) {
        struct Process *current = scheduler.current_process;

        current->exit_code = status;

        signal_notify(current->parent, SIGCHLD);

        for (size_t i = 0; i < current->children_count; ++i) {
                signal_notify(current->children[i], SIGHUP);
        }

        sys_kill(current->pid);

        __asm__("msr    msp, %0\n\r" : : "r"(scheduler.main_kernel_stack));
        context_switch();
}

void sys_kill(const pid_t pid) {
        struct Process *process = &scheduler.processes[pid];
        if (!process->ptr) {
                return;
        }

        for (size_t i = 0; i < process->files.count; ++i) {
                struct File *file = process->files.fdtable[i];

                if (file->f_owner == process->pid) {
                        kfree(file->f_op);
                        kfree(file);
                }
        }

        kfree(process->files.fdtable);
        kfree(process->children);
        deallocate_signal_queue(&process->pending_signals);
        kfree(process->ptr);
        scheduler.total_allocated_memory -= process->allocated_memory;
        process->allocated_memory = 0;

        scheduler.processes[pid].pstate = TERMINATED;
}

void run_process_init(void) {
        scheduler.current_process = &scheduler.processes[0];
        scheduler.current_process->pstate = RUNNING;
        __asm__("movs   r0, #0\n\r"  // run init (pid=0)
                "svc    #0xff\n\r"); // OS_INIT_SVC
}
