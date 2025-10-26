//
// Created by Artur Twardzik on 31/08/2025.
//

#ifndef OS_RESOURCES_H
#define OS_RESOURCES_H

struct wait_queue_entry;

typedef struct wait_queue_entry *wait_queue_head_t;

/**
 * Adds calling process to wait queue. Sets it state to WAITING_FOR_RESOURCE and if the condition is not met
 * forces context switch.
 * @param wq_head head of the waiting queue
 * @param condition function that will return true if the resource is available
 */
void wait_event_interruptible(wait_queue_head_t *wq_head, bool (*condition)(void));

void wake_up_interruptible(wait_queue_head_t *wq_head);

// void wake_up_parent()

#endif //OS_RESOURCES_H
