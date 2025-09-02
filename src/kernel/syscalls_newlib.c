//
// Created by Artur Twardzik on 26/08/2025.
//

#include "printer.h"
#include "memory.h"
#include "file.h"

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


char *__env[1] = {0};
char **environ = __env;

#undef errno
extern int errno;

int _execve(char *name, char **argv, char **env) {
        errno = ENOMEM;
        return -1;
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

int _fork(void) {
        errno = EAGAIN;
        return -1;
}

int _open(const char *name, int flags, int mode) {
        return -1;
}

int _close(int file) {
        return -1;
}


extern char *my_gets(char *ptr, int len);

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

int _kill(int pid, int sig) {
        errno = EINVAL;
        return -1;
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
        return 0;
}

int _wait(int *status) {
        errno = ECHILD;
        return -1;
}
