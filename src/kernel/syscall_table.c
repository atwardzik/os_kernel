//
// Created by Artur Twardzik on 14/11/2025.
//

#include "proc.h"
#include "signal.h"
#include "syscall_codes.h"
#include "fs/file.h"

typedef uint32_t (*syscall_fn)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

syscall_fn syscall_table[] = {
        [EXIT_SVC] = (syscall_fn) sys_exit,
        [SPAWN_SVC] = (syscall_fn) sys_spawn_process,
        [READ_SVC] = (syscall_fn) sys_read,
        [WRITE_SVC] = (syscall_fn) sys_write,
        [OPEN_SVC] = (syscall_fn) sys_open,
        [CLOSE_SVC] = (syscall_fn) sys_close,
        [WAIT_SVC] = (syscall_fn) sys_wait,
        [READDIR_SVC] = (syscall_fn) sys_readdir,
        [CHDIR_SVC] = (syscall_fn) sys_chdir,
        [KILL_SVC] = (syscall_fn) sys_kill,
        [SIGNAL_SVC] = (syscall_fn) sys_signal,
        [SIGRETURN_SVC] = (syscall_fn) sys_sigreturn,
        [LSEEK_SVC] = (syscall_fn) sys_lseek,
};
