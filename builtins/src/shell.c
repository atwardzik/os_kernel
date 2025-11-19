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
                                "\t   (r)un - runs specified app with specified parameters\n"
                                "\t           usage: run path/to/app (parameter)*\n"
                                "\t  (k)ill - kills previously set up by kernel proc with pid=1 (diode)\n"
                                "\t(m)orcik - prints a colorful message\n"
                                "\t  (e)cho - writes to standard input or redirects to a file\n"
                                "\t      cd - change current directory\n"
                                "\t     pwd - print working directory\n\n"
                        );
                }
                else if (strcmp(cmd, "run") == 0 || strcmp(cmd, "r") == 0) {
                        const char *path = strtok(nullptr, " ");
                        char const *path_tok = strtok(nullptr, " ");

                        int fd = open(path, O_BINARY, 0);
                        if (fd < 0) {
                                char buf[64] = {"bin/"};
                                memcpy(buf + 4, path, strlen(path));

                                fd = open(buf, O_BINARY, 0);
                                if (fd < 0) {
                                        puts("Executable not found.\n");
                                        continue;
                                }
                        }

                        const char *path_arg = path_tok ? path_tok : "";

                        char *const program_args[] = {path, path_arg, nullptr};
                        spawn(fd, nullptr, nullptr, program_args, nullptr);

                        int code;
                        const int returned_pid = wait(&code);

                        printf("\n\x1b[96;40m[PATER ADAMVS]\x1b[0m Child process %i exited with code: %i\n",
                               returned_pid, code);
                        close(fd);
                }
                else if (strcmp(cmd, "kill") == 0 || strcmp(cmd, "k") == 0) {
                        printf("You are willing to kill the process pid=1. Choose (1) SIGTERM or (2) SIGKILL: ");

                        char line[80];
                        read(0, &line, sizeof(line));
                        const int choice = line[0] - 0x30;


                        if (choice == 1) {
                                kill(1, 15); //sigterm
                        }
                        else if (choice == 2) {
                                kill(1, 9); //sigkill
                        }
                        else {
                                printf("This is not a valid signal. I won't kill the process.\n");
                        }
                }
                else if (strcmp(cmd, "morcik") == 0 || strcmp(cmd, "m") == 0) {
                        const char *text = "Meine beliebte Olga ist die schoenste Frau auf der Welt";
                        const int colors[] = {31, 91, 33, 32, 34, 94, 35};
                        const int num_colors = sizeof(colors) / sizeof(colors[0]);

                        for (int i = 0; i < strlen(text); ++i) {
                                const int color = colors[i % num_colors];
                                printf("\x1b[%i;40m%c\x1b[0m", color, text[i]);
                        }

                        puts("\n");
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
