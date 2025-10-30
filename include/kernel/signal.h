//
// Created by Artur Twardzik on 28/10/2025.
//

#ifndef OS_SIGNAL_H
#define OS_SIGNAL_H

#include "proc.h"

#include <sys/signal.h>

typedef typeof(void (int)) *sighandler_t;

void init_default_sighandlers(struct Process *process);

void deallocate_signal_queue(signal_queue_head_t *sq_head);

void signal_notify(struct Process *process, int sig);

/**
 * @return The signal if it is pending, -1 on no signals pending.
 */
int get_pending_signal(void);

bool handle_pending_signal(int pending_signal);

void sys_sigreturn(void);

sighandler_t sys_signal(int signum, sighandler_t handler);

#endif //OS_SIGNAL_H
