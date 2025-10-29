//
// Created by Artur Twardzik on 28/10/2025.
//

#ifndef OS_SIGNAL_H
#define OS_SIGNAL_H

#include "proc.h"

#include <sys/signal.h>

void init_default_sighandlers(struct Process *process);

void deallocate_signal_queue(signal_queue_head_t *sq_head);

void signal_notify(struct Process *process, int sig);

void handle_pending_signal();

#endif //OS_SIGNAL_H
