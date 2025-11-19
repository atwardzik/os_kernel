//
// Created by Artur Twardzik on 19/11/2025.
//

#include "libc.h"

#include "syscall_codes.h"

#include <stdarg.h>

void __attribute__((naked)) _start() {
        __asm__("b      main\n\r"
                "svc    #1\n\r");
}

/*
 * Syscalls
 */

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define SYSCALL(syscall_number) __asm__("svc   #" STR(syscall_number) "\n\r");


int write(int file, const void *buf, int len) {
        int res;

        __asm__("svc    #5\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}

int read(int file, void *buf, int len) {
        int res;

        __asm__("svc    #4\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}


void exit(int code) {
        SYSCALL(EXIT_SVC)
}

pid_t spawnp(
        void (*process_entry_ptr)(void),
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
) {
        int ret;
        SYSCALL(SPAWNP_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

pid_t spawn(
        int fd,
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

sighandler_t signal(int signum, sighandler_t handler) {
        sighandler_t ret;
        SYSCALL(SIGNAL_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

pid_t wait(int *stat_loc) {
        int ret;
        SYSCALL(WAIT_SVC)

        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}


int open(const char *name, int flags, int mode) {
        int ret;
        SYSCALL(OPEN_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

int close(int file) {
        int ret;
        SYSCALL(CLOSE_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}


int fstat(int file, struct stat *st) {
        int ret;
        SYSCALL(FSTAT_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

int kill(int pid, int sig) {
        int ret;
        SYSCALL(KILL_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

int lseek(int file, int ptr, int dir) {
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

char *getcwd(char *buf, unsigned int len) {
        char *ret;
        SYSCALL(GETCWD_SVC)
        __asm__("mov    %0, r0\n\r" : "=r"(ret));

        return ret;
}

/*
 * string
 */

int strlen(const char *str) {
        int len = 0;

        while (*(str + len)) {
                len += 1;
        }

        return len;
}

int puts(const char *str) {
        int len = strlen(str);

        return write(1, str, len);
}

int strcspn(const char *str, const char *delims) {
        const int delims_len = strlen(delims);
        int len = 0;


        while (*(str + len)) {
                for (int i = 0; i < delims_len; ++i) {
                        if (*(str + len) == delims[i]) {
                                return len;
                        }
                }
                len += 1;
        }

        return len;
}

int strspn(const char *str, const char *src) {
        const int src_len = strlen(src);
        int len = 0;


        while (*(str + len)) {
                bool contains_src_char = false;
                for (int i = 0; i < src_len; ++i) {
                        if (*(str + len) == src[i]) {
                                contains_src_char = true;
                        }
                }

                if (!contains_src_char) {
                        return len;
                }
                len += 1;
        }

        return len;
}

char *strtok(char *str, const char *delim) {
        static char *token_start = nullptr;
        static char *ptr = nullptr;
        if (str) {
                ptr = str;
        }

        if (!ptr || !*ptr) {
                return nullptr;
        }

        token_start = ptr;
        ptr += strcspn(ptr, delim);

        const int delims_len = strspn(ptr, delim);
        for (int i = 0; i < delims_len; ++i) {
                *ptr = 0;
                ptr += 1;
        }

        return token_start;
}

int strcmp(const char *s1, const char *s2) {
        int offset = 0;

        while (s1[offset] && s2[offset]) {
                if (s1[offset] == s2[offset]) {
                        offset += 1;
                }

                return s1[offset] - s2[offset];
        }

        return 0;
}

char *itoa(int value, char *const str, const int base) {
        if (value == 0) {
                *str = '0';
                return str;
        }

        int i = 0;
        int temp_value = value;
        while (temp_value) {
                i += 1;
                temp_value /= base;
        }

        while (value) {
                const unsigned char digit = (value % base);
                if (digit > 9) {
                        *(str + i - 1) = 'A' + 10 - digit;
                }
                else {
                        *(str + i - 1) = digit + 0x30;
                }

                value /= base;
                i -= 1;
        }

        return str;
}

/**
 * Simple printf allowing %c, %s and %i, %x parameters. Currently unsafe, as the behaviour
 * is undefined with not enough parameters supplied.
 *
 * @param format - formatting string
 * @param ... - parameters
 */
int printf(const char *format, ...) {
        if (strcspn(format, "%") == strlen(format)) {
                puts(format);
                return 0;
        }

        const char *const format_end = format + strlen(format) + 1;

        va_list args;
        va_start(args);

        const char *ptr_begin = format;
        const char *ptr_end = format;
        while (*ptr_end && ptr_end < format_end) {
                ptr_end += strcspn(ptr_begin, "%");

                int len = ptr_end - ptr_begin;
                write(1, ptr_begin, len);

                ptr_end += 1;
                if (ptr_end == format_end) {
                        break;
                }
                switch (*ptr_end) {
                        case 'c': {
                                int c = va_arg(args, int);
                                char buf[1] = {(char) c};
                                write(1, &buf, 1);
                                break;
                        }
                        case 's': {
                                char *str = va_arg(args, const char *);
                                write(1, str, strlen(str));
                                break;
                        }
                        case 'i': {
                                char buf[20];
                                itoa(va_arg(args, int), &buf, 10);
                                write(1, &buf, strlen(buf));
                                break;
                        }
                        case 'x': {
                                char buf[20];
                                itoa(va_arg(args, int), &buf, 16);
                                write(1, &buf, strlen(buf));
                                break;
                        }
                        default:
                                return -1;
                }
                ptr_end += 1;


                ptr_begin = ptr_end;
        }

        va_end(args);
        return 0;
}

void *memcpy(void *dest, const void *src, const unsigned int count) {
        for (unsigned int i = 0; i < count; ++i) {
                *((char *) dest + i) = *((const char *) src + i);
        }

        return dest;
}

void *memset(void *dest, const int ch, const unsigned int count) {
        for (unsigned int i = 0; i < count; ++i) {
                *((char *) dest + i) = (unsigned char) ch;
        }

        return dest;
}
