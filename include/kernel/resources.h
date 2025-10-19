//
// Created by Artur Twardzik on 31/08/2025.
//

#ifndef OS_RESOURCES_H
#define OS_RESOURCES_H

#include "proc.h"
#include "resources_codes.h"

#include <sys/types.h>

typedef uint8_t Resource;


struct wait_queue_entry {
        struct Process *waiting_process;
        struct wait_queue_entry *next;
};

typedef struct wait_queue_entry *wait_queue_head_t;

void add_to_wait_queue(wait_queue_head_t *wq_head, struct Process *process);

void remove_from_wait_queue(wait_queue_head_t *wq_head);

void wait_event_interruptible(wait_queue_head_t *wq_head, bool (*condition)(void));

void wake_up_interruptible(wait_queue_head_t *wq_head);

/**
 * Blocks current process on a specified resource. \n
 * Please note that this function <b>HAS</b> to be run in <b>handler mode</b>.
 * @return Pointer to the resource
 */
void block_resource_on_condition(pid_t parent_process, Resource resource, bool (*condition)(void));

pid_t get_resource_acquiring_process(void);

#endif //OS_RESOURCES_H
