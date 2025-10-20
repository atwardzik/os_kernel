//
// Created by Artur Twardzik on 31/08/2025.
//

#include "resources.h"

#include "proc.h"

//TODO: resource queue to be implemented...
static struct {
        bool none;
        pid_t process;

        bool (*condition)(void);
} process_blocked_on_io_keyboard = {true};

struct wait_queue_entry {
        struct Process *waiting_process;
        struct wait_queue_entry *next;
};

static void add_to_wait_queue(wait_queue_head_t *wq_head, struct Process *process) {
        if (!wq_head) {
                return;
        }

        struct wait_queue_entry *new_entry = kmalloc(sizeof(*new_entry));
        new_entry->waiting_process = process;
        new_entry->next = nullptr;

        if (!*wq_head) {
                *wq_head = new_entry;
                return;
        }

        struct wait_queue_entry *entry = *wq_head;
        while (entry->next) {
                entry = entry->next;
        }

        entry->next = new_entry;

        process->pstate = WAITING_FOR_RESOURCE;
}

static struct Process *pop_from_wait_queue(wait_queue_head_t *wq_head) {
        if (!wq_head || !*wq_head) {
                return nullptr;
        }

        struct wait_queue_entry *old_head = *wq_head;
        *wq_head = old_head->next;

        struct Process *head_process = old_head->waiting_process;
        kfree(old_head);

        return head_process;
}

void wait_event_interruptible(wait_queue_head_t *wq_head, bool (*condition)(void)) {
        struct Process *process = scheduler_get_current_process();
        add_to_wait_queue(wq_head, process);
        process->pstate = WAITING_FOR_RESOURCE;

        if (!condition()) {
                context_switch_from_kernel();
        }
}

void wake_up_interruptible(wait_queue_head_t *wq_head) {
        struct Process *head_process = pop_from_wait_queue(wq_head);

        if (head_process) {
                head_process->pstate = READY;
        }
}

/**
 * Find the process waiting for the resource with the highest priority
 *
 * @param resource code of the resource
 * @return PID of the resource or PID_NO_SUCH_PROCESS=0xffff
 */
static pid_t get_process_waiting_for_resource(const Resource resource) {
        if (resource == IO_KEYBOARD && process_blocked_on_io_keyboard.none == false) {
                const int stream_offset = process_blocked_on_io_keyboard.condition();

                if (stream_offset) {
                        const pid_t waiting_process = process_blocked_on_io_keyboard.process;
                        process_blocked_on_io_keyboard.none = true;

                        change_process_state(waiting_process, READY);
                        return waiting_process;
                }
        }

        return PID_NO_SUCH_PROCESS;
}

void block_resource_on_condition(pid_t parent_process, Resource resource, bool (*condition)()) {
        if (resource == IO_KEYBOARD && process_blocked_on_io_keyboard.none == true) {
                change_process_state(parent_process, WAITING_FOR_RESOURCE);
                process_blocked_on_io_keyboard.process = parent_process;
                process_blocked_on_io_keyboard.none = false;

                process_blocked_on_io_keyboard.condition = condition;
        }
}

pid_t get_resource_acquiring_process() {
        return get_process_waiting_for_resource(IO_KEYBOARD); //TODO: run through all blocked resources
}
