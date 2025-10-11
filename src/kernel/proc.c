//
// Created by Artur Twardzik on 30/12/2024.
//

#include "proc.h"

#include <stdio.h>

#include "drivers/divider.h"

static constexpr size_t MAX_PROCESS_NUMBER = 20; //TODO: change to dynamic processes count
static constexpr int HASH_MODULO = 3;            //has to be coprime with MAX_PROCESS_NUMBER
static constexpr int DELETED_TRACER = MAX_PROCESS_NUMBER * 2;

static constexpr int DEFAULT_PROCESS_SIZE = 4 * 1024; //4 [KB]

#define THREAD_PSP_CODE ((void *) 0xfffffffd);

static struct {
        struct Process processes[MAX_PROCESS_NUMBER];
        size_t total_allocated_memory;
        pid_t current_process;
} scheduler __attribute__ ((section (".data")));

extern void init_process_stack_frame(void **initial_sp, uint32_t xpsr, uint32_t pc, uint32_t lr);

extern uint32_t save_state(void **process_stack);

extern void recall_state(void *sp);


static size_t calculate_pid_hash(pid_t pid, size_t i) {
        static size_t pid_hash[MAX_PROCESS_NUMBER];
        const size_t index = pid % 20;

        if (!pid_hash[index] || pid_hash[index] == DELETED_TRACER || pid == 0) {
                pid_hash[index] = pid;
                return index;
        }

        return calculate_pid_hash(pid + HASH_MODULO * i, i + 1);
}

static size_t get_pid_position(pid_t pid) {
        return calculate_pid_hash(pid, 0);
}

void scheduler_init(void) {
        for (size_t i = 0; i < MAX_PROCESS_NUMBER; ++i) {
                scheduler.processes[i].ptr = nullptr;
        }

        scheduler.total_allocated_memory = 0;
}

pid_t scheduler_get_current_process() {
        return scheduler.current_process;
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

static size_t current_index = 0;

void *get_next_process() {
        do {
                //TODO: determine task importance, also by implementing priority queue
                current_index += 1;
                if (current_index == MAX_PROCESS_NUMBER) {
                        current_index = 0;
                }
        } while (scheduler.processes[current_index].pstate != READY);

        scheduler.current_process = scheduler.processes[current_index].pid;

        scheduler.processes[current_index].pstate = RUNNING;
        return scheduler.processes[current_index].pstack;
}

void update_process(void *psp) {
        scheduler.processes[current_index].pstate = READY;
        scheduler.processes[current_index].pstack = psp;
}

void *update_process_and_get_next(void *psp) {
        update_process(psp);

        return get_next_process();
}

pid_t create_process(void (*process_entry_ptr)(void)) {
        static pid_t pid = 0;

        if (pid == MAX_PROCESS_NUMBER) {
                __asm__("bkpt   #0");
        }

        void *process_page = kmalloc(DEFAULT_PROCESS_SIZE);
        void *pstack = process_page + DEFAULT_PROCESS_SIZE - sizeof(size_t);
        init_process_stack_frame(&pstack, 0x0100'0000, (uint32_t) process_entry_ptr, 0xfffffffd);

        const struct Process process = {process_page, pstack, pid, READY, DEFAULT_PROCESS_SIZE};
        scheduler.processes[pid] = process;

        pid += 1;
        return process.pid;
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
        __asm__("movs   r0, #0\n\r" // run process 0
                "movs   r7, #255\n\r"
                "svc    #0\n\r");
        // systick_enable(625'000);
}
