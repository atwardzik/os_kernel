//
// Created by Artur Twardzik on 30/12/2024.
//

#include "proc.h"

static struct {
        struct Process *processes;
        size_t total_allocated_memory;
        size_t maximum_processes_size;
        pid_t current_task;
} scheduler __attribute__ ((section (".data")));

void scheduler_init(void) {
        scheduler.processes = kmalloc(sizeof(struct Process) * 4);
        scheduler.total_allocated_memory = 0;
        scheduler.maximum_processes_size = HEAP_SIZE - get_current_heap_size();
        scheduler.current_task = 0;
}
