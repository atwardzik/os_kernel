#if 0
#define O_BINARY 0x10000
#define O_RDONLY 0
#define O_WRONLY 1
#define SEEK_SET 0
#define O_DIRECTORY 0x200000

constexpr int MAX_FILENAME_LEN = 32;

struct DirectoryEntry {
        unsigned char file_type;
        int inode_index;
        char name[MAX_FILENAME_LEN];
};


void __attribute__((naked)) _start() {
        __asm__("b      main\n\r"
                "svc    #1\n\r");
}

int write(int, const void *, int) {
        int res;

        __asm__("svc    #4\n\r");
        __asm__("mov    %0, r0\n\r" : "=r"(res));

        return res;
}

int read(int file, void *ptr, int len) {
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
#endif
#if 0
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


extern int printf(const char *fmt, ...);
extern ssize_t read(int fd, void *buf, size_t count);
extern ssize_t write(int fd, void *buf, size_t count);
extern int open(const char *, int, ...);
extern int close(int __fildes);
#endif

#if 0

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
#endif

#include "libc.h"

int main(void) {
        while (1) {
                puts(" > ");
                char buffer[256];
                if (read(0, buffer, 256) < 1) {
                        continue;
                }
                buffer[strcspn(buffer, "\n")] = 0;

                char *cmd = strtok(buffer, " ");

                if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
                        puts("Temporarily available commands:\n"
                             "\t  (h)elp - prints this help screen\n"
                             "\t   (r)un - runs specified app, if no argument is provided runs proc2\n"
                             "\t           (standard input test). Proc1 is already running (diode)\n"
                             "\t  (k)ill - kills proc1 (diode)\n"
                             "\t(m)orcik - prints a colorful message\n"
                             "\t       -----\n"
                             "\t      ls - lists current directory\n"
                             "\t   mkdir - creates a directory under specified path\n"
                             "\t   touch - creates a file under specified path\n"
                             "\t     cat - reads file contents\n"
                             "\t  (e)cho - writes to standard input or redirects to a file\n"
                             "\t rawecho - writes converted hexadecimal bytes to specified file (max 128 bytes)\n\n");
                }
                else if (strcmp(cmd, "run") == 0 || strcmp(cmd, "r") == 0) {
                        const char *path = strtok(nullptr, " ");

                        int fd = open(path, O_BINARY, 0);
                        if (fd < 0) {
                                puts("Executable not found.\n");
                                continue;
                        }

                        short raw_bytes_app[128];
                        read(fd, raw_bytes_app, 128);
                        // spawn((void *) raw_bytes_app + 1, nullptr, nullptr, nullptr, nullptr);

                        int code = -1;
                        const int returned_pid = -1;
                        // const int returned_pid = wait(&code);

                        puts("\n\x1b[96;40m[PATER ADAMVS]\x1b[0m Child process ");

                        char num[5];
                        itoa(returned_pid, num, 10);
                        puts(num);

                        puts("exited with code: ");

                        itoa(code, num, 10);
                        puts(num);

                        puts("\n");
                }
                else if (strcmp(cmd, "kill") == 0 || strcmp(cmd, "k") == 0) {
                        puts("N/a");
                }
                else if (strcmp(cmd, "morcik") == 0 || strcmp(cmd, "m") == 0) {
                        puts("\x1b[95;40mMeine beliebte Olga ist die sch\xf6nste Frau auf der Welt\n\x1b[0m");
                }
                else if (strcmp(cmd, "echo") == 0 || strcmp(cmd, "e") == 0) {
                        const char *text = strtok(nullptr, ">");
                        const char *path = strtok(nullptr, "");
                        if (!text) {
                                continue;
                        }

                        if (!path) {
                                puts(text);
                                continue;
                        }

                        int fd = open(path, O_WRONLY, 0);
                        if (fd < 0) {
                                puts("No such file.\n");
                                continue;
                        }

                        write(fd, text, strlen(text) + 1); // with EOF

                        close(fd);
                }
                else if (strcmp(cmd, "ls") == 0) {
                        const char *path_tok = strtok(nullptr, " ");
                        const char *path = path_tok == nullptr ? "" : path_tok;
                        const int dirfd = open(path, O_DIRECTORY | O_RDONLY, 0);

                        if (dirfd < 0) {
                                puts("No such file.");
                                continue;
                        }

                        struct DirectoryEntry dentry;
                        while (readdir(dirfd, &dentry) == 1) {
                                puts((const char *) &dentry.file_type);
                                puts(" ");
                                puts(dentry.name);
                                puts("\n");
                        }

                        lseek(dirfd, 0, SEEK_SET);
                }
                else if (strcmp(cmd, "cd") == 0) {
                        const char *path = strtok(nullptr, " ");

                        const int code = chdir(path);
                        if (code == -1) {
                                puts("No such file.\n");
                        }
                }
                else if (strcmp(cmd, "pwd") == 0) {
                        char buf[64];
                        char *ptr = buf;
                        char *ret = getcwd(ptr, 64);

                        puts(ret);
                        puts("\n");
                }
        }
}
