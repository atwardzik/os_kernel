//
// Created by Artur Twardzik on 26/08/2025.
//

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "printer.h"

extern uint8_t __heap_start__[];
extern uint8_t __heap_end__[];

static uint8_t *const heap_start_ptr = __heap_start__;
static uint8_t *const heap_end_ptr = __heap_end__;
// static const uint8_t heap_length = *__heap_length;

char *__env[1] = {0};
char **environ = __env;

#undef errno
extern int errno;

int _execve(char *name, char **argv, char **env) {
        errno = ENOMEM;
        return -1;
}

caddr_t _sbrk(int incr) {
        static uint8_t *current_bump_address = heap_start_ptr;
        uint8_t *prev_bump_addr;

        prev_bump_addr = current_bump_address;
        if (current_bump_address + incr > heap_end_ptr) {
                write(1, "Heap and stack collision\n", 25);
                // abort ();
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

int _read(int file, char *ptr, int len) {
        return 0;
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

int _kill(int pid, int sig) {
        errno = EINVAL;
        return -1;
}

int _lseek(int file, int ptr, int dir) {
        return 0;
}

int _write(int file, char *ptr, int len) {
        // for (int i = 0; i < len; i++) {
        // write_byte(*ptr++);
        // }

        write_string(ptr);

        return len;
}
