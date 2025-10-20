//
// Created by Artur Twardzik on 30/12/2024.
//

#include "proc.h"

#include "tty.h"
#include "fs/file.h"

#include <stdio.h>


static constexpr size_t MAX_PROCESS_NUMBER = 20; //TODO: change to dynamic processes count

static constexpr int DEFAULT_PROCESS_SIZE = 8 * 1024; //8 [KB]

static constexpr off_t DEFAULT_PROCESS_SP_OFFSET = 6 * 1024;

#define THREAD_PSP_CODE ((void *) 0xfffffffd);

static struct {
        struct Process processes[MAX_PROCESS_NUMBER];
        size_t total_allocated_memory;
        pid_t current_process;

        void *main_kernel_stack; //TODO: see if it is needed after processes start running
} scheduler __attribute__ ((section (".data")));

extern void init_process_stack_frame(void **initial_sp, uint32_t xpsr, uint32_t pc, uint32_t lr);

extern uint32_t save_state(void **process_stack);

extern void recall_state(void *sp);

void scheduler_init(void) {
        for (size_t i = 0; i < MAX_PROCESS_NUMBER; ++i) {
                scheduler.processes[i].ptr = nullptr;
        }

        scheduler.total_allocated_memory = 0;
        scheduler.current_process = PID_NO_SUCH_PROCESS;

        __asm__("mrs %0, msp\n\t" : "=r" (scheduler.main_kernel_stack));
}

struct Process *scheduler_get_current_process() {
        return &scheduler.processes[scheduler.current_process];
}

void **scheduler_get_kernel_stack(void) {
        if (scheduler.current_process == PID_NO_SUCH_PROCESS) {
                return nullptr;
        }

        return &scheduler.processes[scheduler.current_process].kstack;
}

void **scheduler_get_process_stack(const pid_t current_process) {
        size_t index = 0;
        while (scheduler.processes[index].pid != current_process) {
                index += 1;

                if (index == MAX_PROCESS_NUMBER) {
                        return nullptr;
                }
        }

        return &scheduler.processes[index].pstack;
}


void *get_next_process() {
        static size_t current_index = 0;
        do {
                //TODO: determine task importance, also by implementing priority queue
                current_index += 1;
                if (current_index == MAX_PROCESS_NUMBER) {
                        current_index = 0;
                }
        } while (scheduler.processes[current_index].pstate != READY);

        scheduler.current_process = current_index;

        scheduler.processes[current_index].pstate = RUNNING;


        if (scheduler.processes[current_index].kernel_mode) {
                __asm__("msr    psp, %0" : : "r" (scheduler.processes[current_index].pstack));
                return scheduler.processes[current_index].kstack;
        }
        __asm__("msr    msp, %0" : : "r" (scheduler.processes[current_index].kstack));
        return scheduler.processes[current_index].pstack;
}

void update_process(void *psp, void *msp) {
        if (!scheduler.processes[scheduler.current_process].kernel_mode) {
                scheduler.processes[scheduler.current_process].pstate = READY;
        }

        scheduler.processes[scheduler.current_process].pstack = psp;
        scheduler.processes[scheduler.current_process].kstack = msp;
}

void *update_process_and_get_next(void *psp, void *msp) {
        update_process(psp, msp);

        return get_next_process();
}

pid_t create_process(void (*process_entry_ptr)(void)) {
        static pid_t pid = 0;

        if (pid == MAX_PROCESS_NUMBER) {
                __asm__("bkpt   #0");
        }

        void *process_page = kmalloc(DEFAULT_PROCESS_SIZE);
        void *kstack = process_page + DEFAULT_PROCESS_SIZE - sizeof(size_t);
        void *pstack = process_page + DEFAULT_PROCESS_SP_OFFSET - sizeof(size_t);
        init_process_stack_frame(&pstack, 0x0100'0000, (uint32_t) process_entry_ptr, 0xfffffffd);


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

bool is_in_kernel_mode() {
        return scheduler.processes[scheduler.current_process].kernel_mode;
}

void set_kernel_mode_flag() {
        scheduler.processes[scheduler.current_process].kernel_mode = true;
}

void reset_kernel_mode_flag() {
        scheduler.processes[scheduler.current_process].kernel_mode = false;
}

void change_process_state(const pid_t process, const enum State state) {
        for (size_t i = 0; i < MAX_PROCESS_NUMBER; ++i) {
                if (scheduler.processes[i].pid == process) {
                        scheduler.processes[i].pstate = state;
                }
        }
}

int __attribute__((naked)) exit() {
        __asm__("bkpt   #0");
}

extern void systick_enable(uint32_t cycles);

void run_all_processes(void) {
        scheduler.current_process = 0;
        scheduler.processes[scheduler.current_process].pstate = RUNNING;
        __asm__("movs   r0, #0\n\r" // run process 0
                "movs   r7, #255\n\r"
                "svc    #0\n\r");
        // systick_enable(625'000);
}
