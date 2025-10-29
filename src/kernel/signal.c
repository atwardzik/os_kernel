//
// Created by Artur Twardzik on 28/10/2025.
//

#include "signal.h"

#include "memory.h"

struct signal_queue_entry {
        int pending_signal;

        struct signal_queue_entry *next;
};

static void add_to_signal_queue(signal_queue_head_t *sq_head, const int signal) {
        if (!sq_head) {
                return;
        }

        struct signal_queue_entry *new_entry = kmalloc(sizeof(*new_entry));
        new_entry->pending_signal = signal;
        new_entry->next = nullptr;

        if (!*sq_head) {
                *sq_head = new_entry;
                return;
        }

        struct signal_queue_entry *entry = *sq_head;
        while (entry->next) {
                entry = entry->next;
        }

        entry->next = new_entry;
}

static int pop_from_signal_queue(signal_queue_head_t *sq_head) {
        if (!sq_head || !*sq_head) {
                return -1;
        }

        struct signal_queue_entry *old_head = *sq_head;
        *sq_head = old_head->next;

        const int pending = old_head->pending_signal;
        kfree(old_head);

        return pending;
}

static int top_from_signal_queue(const signal_queue_head_t *sq_head) {
        struct signal_queue_entry const *head = *sq_head;

        return head->pending_signal;
}

void deallocate_signal_queue(signal_queue_head_t *sq_head) {
        if (!sq_head) {

        }

        while (*sq_head) {
                pop_from_signal_queue(sq_head);
        }
}

[[noreturn]] static void action_terminate() {
        //Termination due to an uncaught signal results in exit status 128+[<signal number>]
        struct Process *current_process = scheduler_get_current_process();
        const int pending = pop_from_signal_queue(&current_process->pending_signals);

        sys_exit(128 + pending);
}

static void action_ignore() {
        //As the name suggests, this ignores the signal.
}


void init_default_sighandlers(struct Process *process) {
        process->sighandlers[SIGTERM] = action_terminate;
        process->sighandlers[SIGKILL] = action_terminate;
        process->sighandlers[SIGINT] = action_terminate;
        process->sighandlers[SIGHUP] = action_terminate;

        process->sighandlers[SIGCHLD] = action_ignore;
}

void signal_notify(struct Process *process, const int sig) {
        if (!(process->signal_mask & (1 << sig))) {
                add_to_signal_queue(&process->pending_signals, sig);
        }
}

void handle_pending_signal() {
        struct Process *current_process = scheduler_get_current_process();
        const int pending = top_from_signal_queue(&current_process->pending_signals);

        if (pending < 0 || pending >= 32) {
                return;
        }

        if (current_process->sighandlers[pending]) {
                (*current_process->sighandlers[pending])();
        }

        pop_from_signal_queue(&current_process->pending_signals);
}
