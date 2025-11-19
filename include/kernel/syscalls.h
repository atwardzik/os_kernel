//
// Created by Artur Twardzik on 25/10/2025.
//

#ifndef OS_SYSCALLS_H
#define OS_SYSCALLS_H

#include "proc.h"
#include "types.h"
#include "fs/file.h"

/**
 * Function for spawning processes from some known-address
 *
 * @param process_entry_ptr
 * @param attrp
 * @param argv
 * @param envp
 * @return
 */
pid_t spawnp(
        void (*process_entry_ptr)(void),
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
);

/**
 * Main spawn function, for running programs from selected file.
 *
 * @param fd File descriptor of a file containing executable
 * @param file_actions
 * @param attrp 
 * @param argv points to the arguments list
 * @param envp 
 * @return pid of a spawned process
 */
pid_t spawn(
        int fd,
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
);

[[noreturn]] void sigreturn(void);

int kill(int pid, int sig);

int readdir(int dirfd, struct DirectoryEntry *directory_entry);

int chdir(const char *path);

#endif //OS_SYSCALLS_H
