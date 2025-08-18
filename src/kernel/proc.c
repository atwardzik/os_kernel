//
// Created by Artur Twardzik on 30/12/2024.
//

#include "proc.h"
#include "drivers/divider.h"

static constexpr size_t MAX_PROCESS_NUMBER = 20; //TODO: change to dynamic processes count
static constexpr int HASH_MODULO = 3; //has to be coprime with MAX_PROCESS_NUMBER
static constexpr int DELETED_TRACER = MAX_PROCESS_NUMBER * 2;

static constexpr int DEFAULT_PROCESS_SIZE = 3 * 1024; //3 [KB]

#define THREAD_PSP_CODE ((void *) 0xfffffffd);

static struct {
        struct Process *processes;
        size_t total_allocated_memory;
        size_t maximum_processes_size;
        pid_t current_task;
        pid_t last_pid;
} scheduler __attribute__ ((section (".data")));


static size_t calculate_pid_hash(pid_t pid, size_t i) {
        static size_t pid_hash[MAX_PROCESS_NUMBER];
        size_t index = hw_mod(pid, 20);

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
        scheduler.maximum_processes_size = HEAP_SIZE - get_current_heap_size();
        scheduler.current_task = 0;
        scheduler.last_pid = 0;
}

pid_t create_process(void (*process_start_ptr)(void)) {
        pid_t pid = scheduler.last_pid;
        scheduler.last_pid += 1;

        size_t index = get_pid_position(pid);

        void *process_page = kmalloc(DEFAULT_PROCESS_SIZE);
        void *pstack = process_page + DEFAULT_PROCESS_SIZE;

        /* Save special registers which will be restored on exc. return:
           - XPSR: Default value (0x01000000)
           - PC: Point to the handler function
           - LR: Point to a function to be called when the handler returns */
        *(uint32_t *) (pstack - 1) = 0x0100'0000;
        *(uint32_t *) (pstack - 2) = (uint32_t) process_start_ptr;
        *(uint32_t *) (pstack - 3) = (uint32_t) exit;
        //further registers ...

        pstack -= 16;
        struct Process process = {process_page, pstack, pid, ready, DEFAULT_PROCESS_SIZE};
        scheduler.processes[index] = process;

        return pid;
}

void exit(int exit_code) {}
