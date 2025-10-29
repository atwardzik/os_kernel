//
// Created by Artur Twardzik on 25/10/2025.
//

#ifndef OS_SYSCALLS_H
#define OS_SYSCALLS_H

#include "proc.h"
#include "types.h"

pid_t spawn(
        void (*process_entry_ptr)(void),
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
);

[[noreturn]] void sigreturn(void);

#endif //OS_SYSCALLS_H
