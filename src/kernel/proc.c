//
// Created by Artur Twardzik on 30/12/2024.
//

#include "proc.h"
#include "drivers/divider.h"

static constexpr size_t MAX_PROCESS_NUMBER = 20; //TODO: change to dynamic processes count
static constexpr int HASH_MODULO = 3;            //has to be coprime with MAX_PROCESS_NUMBER
static constexpr int DELETED_TRACER = MAX_PROCESS_NUMBER * 2;

static constexpr int DEFAULT_PROCESS_SIZE = 4 * 1024; //4 [KB]

#define THREAD_PSP_CODE ((void *) 0xfffffffd);

static struct {
        struct Process *processes;
        size_t total_allocated_memory;
        pid_t current_task;
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
        scheduler.processes = kmalloc(sizeof(struct Process) * MAX_PROCESS_NUMBER);
        scheduler.total_allocated_memory = 0;
}

void context_switch(void *current_sp) {
        static size_t index = 0;


        if (scheduler.processes[index].pstate == RUNNING) {
                //only if the process is said to be RUNNING change it to be suspended.
                //some syscalls may change current process state to be e.g. WAITING_FOR_RESOURCE
                scheduler.processes[index].pstate = SUSPENDED;
                scheduler.processes[index].pstack = current_sp;
        }

        if (scheduler.processes[index].pstate == NEW) {
                scheduler.processes[index].pstate = RUNNING;
                recall_state(scheduler.processes[index].pstack);

                return;
        }

        save_state(&scheduler.processes[index].pstack);
        do {
                //TODO: determine task importance, also by implementing priority queue
                index += 1;
                if (index % MAX_PROCESS_NUMBER == 0) {
                        index = 0;
                }
        } while (!scheduler.processes[index].ptr);

        scheduler.processes[index].pstate = RUNNING;
        recall_state(scheduler.processes[index].pstack); //will not exit!
}

pid_t create_process(void (*process_entry_ptr)(void)) {
        static pid_t pid = 0;

        const size_t index = get_pid_position(pid);

        void *process_page = kmalloc(DEFAULT_PROCESS_SIZE);
        void *pstack = process_page + DEFAULT_PROCESS_SIZE;
        init_process_stack_frame(&pstack, 0x0100'0000, (uint32_t) process_entry_ptr, 0xfffffffd);

        struct Process process = {process_page, pstack, pid, NEW, DEFAULT_PROCESS_SIZE};
        scheduler.processes[index] = process;

        pid += 1;
        return pid;
}

int __attribute__((naked)) exit() {
        __asm__("bkpt   #0");
}

extern void systick_enable(uint32_t cycles);

void run_all_processes(void) {
        systick_enable(625'000);
}
