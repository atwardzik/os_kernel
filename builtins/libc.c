//
// Created by Artur Twardzik on 19/11/2025.
//

#include "libc.h"

void __attribute__((naked)) _start() {
        __asm__("b      main\n\r"
                "svc    #1\n\r");
}

/*
 * Syscalls
 */

int write(int file, const void *buf, int len) {
        int res;

        __asm__("svc    #4\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}

int read(int file, void *buf, int len) {
        int res;

        __asm__("svc    #3\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}

int open(const char *name, int flags, int mode) {
        int res;

        __asm__("svc    #5\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}

int close(int file) {
        int res;

        __asm__("svc    #6\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}

int readdir(int dirfd, struct DirectoryEntry *directory_entry) {
        int res;

        __asm__("svc    #8\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}

int chdir(const char *path) {
        int res;

        __asm__("svc    #9\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}

int lseek(const int file, int offset, int whence) {
        int res;

        __asm__("svc    #10\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}

char *getcwd(char *buf, unsigned int len) {
        char *res;

        __asm__("svc    #12\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
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
