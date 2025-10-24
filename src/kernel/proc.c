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


static void *scheduler_get_next_process() {
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
                __asm__("msr    psp, %0" : : "r"(scheduler.current_process->pstack));
                return scheduler.current_process->kstack;
        }
        else {
                __asm__("msr    msp, %0" : : "r"(scheduler.current_process->kstack));
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

pid_t create_process(void (*process_entry_ptr)(void)) {
        static pid_t pid = 0;

        if (pid == scheduler.max_processes) {
                __asm__("bkpt   #0");
        }

        void *process_page = kmalloc(DEFAULT_PROCESS_SIZE);
        void *kstack = process_page + DEFAULT_PROCESS_SIZE - sizeof(size_t);
        void *pstack = process_page + DEFAULT_PROCESS_SP_OFFSET - sizeof(size_t);
        init_process_stack_frame(&pstack, 0x0100'0000, process_entry_ptr, THREAD_PSP_CODE);


        const struct Process process = {
                .ptr = process_page,
                .pstack = pstack,
                .pid = pid,
                .pstate = READY,
                .allocated_memory = DEFAULT_PROCESS_SIZE,
                .priority_level = 0,
                .files = create_tty_file_mock(),
                .kernel_mode = false,
                .kstack = kstack
        };
        scheduler.processes[pid] = process;
        scheduler.total_allocated_memory += DEFAULT_PROCESS_SIZE;

        pid += 1;
        return process.pid;
}

bool is_in_kernel_mode() { return scheduler.current_process->kernel_mode; }

void set_kernel_mode_flag() { scheduler.current_process->kernel_mode = true; }

void reset_kernel_mode_flag() { scheduler.current_process->kernel_mode = false; }

int __attribute__((naked)) sys_exit(int status) { __asm__("bkpt   #0"); }

void run_process_init(void) {
        scheduler.current_process = &scheduler.processes[0];
        scheduler.current_process->pstate = RUNNING;
        __asm__("movs   r0, #0\n\r"  // run init (pid=0)
                "svc    #0xff\n\r"); // OS_INIT_SVC
}
