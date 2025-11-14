//
// Created by Artur Twardzik on 26/08/2025.
//

#include "syscalls.h"

#include "memory.h"
#include "proc.h"
#include "syscall_codes.h"
#include "fs/file.h"

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>


char *__env[1] = {0};
char **environ = __env;

#undef errno
extern int errno;

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define SYSCALL(syscall_number) __asm__("svc   #" STR(syscall_number) "\n\r");

void _exit(int code) {
        SYSCALL(EXIT_SVC)
}

[[deprecated("Not supported, use spawn instead")]]
int _execve(char *name, char **argv, char **env) {
        errno = ENOTSUP;
        return -1;
}

[[deprecated("Not supported, use spawn instead")]]
int _fork(void) {
        errno = ENOTSUP;
        return -1;
}

pid_t spawn(
        void (*process_entry_ptr)(void),
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
) {
        int ret;
        SYSCALL(SPAWN_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

void sigreturn(void) {
        SYSCALL(SIGRETURN_SVC)
}

typedef typeof(void (int)) *sighandler_t;

sighandler_t signal(int signum, sighandler_t handler) {
        SYSCALL(SIGNAL_SVC)
}

pid_t _wait(int *stat_loc) {
        SYSCALL(WAIT_SVC)
}

caddr_t _sbrk(int incr) {
        static uint8_t *current_bump_address = __heap_start__;
        uint8_t *prev_bump_addr;

        prev_bump_addr = current_bump_address;
        if (current_bump_address + incr > user_space_heap_start_ptr) {
                sys_write(1, "[!] User and kernel space heap collision\n", 40);
                __asm__("bkpt   #0");
                // abort();
        }

        current_bump_address += incr;
        return (caddr_t) prev_bump_addr;
}


int _open(const char *name, int flags, int mode) {
        int ret;
        SYSCALL(OPEN_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

int _close(int file) {
        return -1;
}


int _fstat(char *file, struct stat *st) {
        st->st_mode = S_IFCHR;
        return 0;
}

int _stat(char *file, struct stat *st) {
        st->st_mode = S_IFCHR;
        return 0;
}

int _times(struct tms *buf) {
        return -1;
}

int _isatty(int file) {
        return 1;
}

int _getpid(void) {
        return 1;
}

int kill(int pid, int sig) {
        SYSCALL(KILL_SVC)
}

int _kill(int pid, int sig) {
        kill(pid, sig);
}

int _link(char *old, char *new) {
        errno = EMLINK;
        return -1;
}

int _unlink(char *name) {
        errno = ENOENT;
        return -1;
}

int _lseek(int file, int ptr, int dir) {
        int ret;
        SYSCALL(LSEEK_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

int readdir(int dirfd, struct DirectoryEntry *directory_entry) {
        int ret;
        SYSCALL(READDIR_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

int chdir(const char *path) {
        int ret;
        SYSCALL(CHDIR_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}
