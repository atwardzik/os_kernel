#include "libc.h"
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
