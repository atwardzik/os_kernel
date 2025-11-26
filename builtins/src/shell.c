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
                        puts("By default, some crucial executables are stored in /bin and /sbin directories,\n"
                                "they do not require specifying full path. One can ls those directories to\n"
                                "find out what programs are available. For now there is no PATH variable support.\n"
                                "In order to run any scripts/executables use: \n"
                                "\t path/to/exec [parameters]*\n\n"
                                "\tTemporarily available gsh builtin commands:\n"
                                "\t  (h)elp - prints this help screen\n"
                                "\t  (k)ill - kills proc previously set up by kernel with pid=1 (diode)\n"
                                "\t(m)orcik - prints a colorful message\n"
                                "\t  (e)cho - writes to specified file (type echo --h to get more info)\n"
                                "\t rawecho - writes raw hex bytes to specified file NOT SAFE\n"
                                "\t           DO NOT TRY TO PASS NON-HEX BYTES!!!\n"
                                "\t      cd - change current directory\n"
                                "\t     pwd - print working directory\n\n"
                        );
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
                else if (strcmp(cmd, "rawecho") == 0) {
                        const char *text = strtok(nullptr, ">");
                        const char *path = strtok(nullptr, "");
                        if (!text || !path) {
                                continue;
                        }

                        int fd = open(path, O_BINARY, 0);
                        if (fd < 0) {
                                puts("No such file.\n");
                                continue;
                        }

                        char bytes[512] = {};
                        memset(bytes, 0, 512);
                        int index = 0;

                        const char *ptr = text;
                        char *endptr;
                        while (*ptr) {
                                while (*ptr == ' ') {
                                        ptr += 1;
                                }

                                if (*ptr == '\0') {
                                        break;
                                }

                                const unsigned char value = 16 * (*ptr) + *(ptr + 1);

                                bytes[index++] = value;

                                ptr += 2;
                        }

                        write(fd, bytes, index);

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
                else {
                        //it is crucial to check if it is an executable
                        //dummy solution
                        char const *arg1 = strtok(nullptr, " ");
                        char const *arg2 = strtok(nullptr, " ");
                        char const *arg3 = strtok(nullptr, " ");
                        char const *arg4 = strtok(nullptr, " ");
                        char const *arg5 = strtok(nullptr, " ");

                        int fd = open(cmd, O_BINARY, 0);
                        if (fd < 0) {
                                char buf[64] = {"bin/"};
                                memcpy(buf + 4, cmd, strlen(cmd));

                                fd = open(buf, O_BINARY, 0);
                                if (fd < 0) {
                                        puts("Executable not found.\n");
                                        continue;
                                }
                        }

                        char *const program_args[] = {cmd, arg1, arg2, arg3, arg4, arg5, nullptr};
                        spawn(fd, nullptr, nullptr, program_args, nullptr);

                        int code;
                        const int returned_pid = wait(&code);

                        printf("\nChild process %i exited with code: %i\n",
                               returned_pid, code);
                        close(fd);
                }
        }
}

#if 0
void perform_kill() {
        char *sig = nullptr;
        // char *process = nullptr;

        char *param = strtok(nullptr, " ");
        if (strcmp(param, "--h") == 0) {
                puts("kill - terminate or signal a process\n"
                        "Usage:\tkill [-s signal_name] pid\n\n"
                        "Commonly used signals:\n"
                        " - HUP (hang up)\n"
                        " - INT (interrupt)\n"
                        " - QUIT (quit)\n"
                        " - KILL (non-ignorable kill)\n"
                        " - TERM (software termination signal)\n"
                        "By default, without specifying -s parameter, SIGTERM is sent.\n");
        }
        else if (strcmp(param, "-s") == 0) {
                sig = strtok(nullptr, " ");
                // process = strtok(nullptr, " ");
        }
        else {
                // process = strtok(nullptr, " ");
        }

#if 0
pid_t pid = strtol();
        if (pid<1) {
                //failure
        }
#endif
const pid_t pid = 1;

        if (sig== nullptr) {
                kill(pid, 15);
                return;
        }
        else if (strcmp(sig, "TERM")== 0) {
                kill(pid, 15);
        }
        else if (strcmp(sig, "KILL")== 0) {
                kill(pid, 9);
        }
        else {
                printf("This is not a valid signal. I won't kill the process.\n");
        }
}

void run_program(const char *cmd) {
        const char *path = strtok(nullptr, " ");
        char const *arg_tok = strtok(nullptr, " ");
        // find if the program starts with dot slash
        //find multiple arguments
        //find if there is an ampersand to get background processes (memchr)

        int fd = open(path, O_BINARY, 0);
        if (fd < 0) {
                char buf[64] = {"bin/"};
                memcpy(buf + 4, path, strlen(path));

                fd = open(buf, O_BINARY, 0);
                if (fd < 0) {
                        printf("gsh: command not found: %s\n", cmd);
                        return;
                }
        }

        const char *arg = arg_tok ? arg_tok : "";

        char *const program_args[] = {path, arg, nullptr};
        spawn(fd, nullptr, nullptr, program_args, nullptr);

        int code;
        const int returned_pid = wait(&code);

        printf("Child process %i exited with code: %i\n", returned_pid, code);
        close(fd);
}

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
                        puts("By default, some crucial executables are stored in /bin and /sbin directories,"
                                "they do not require specifying full path. One can ls those directories to"
                                "find out what programs are available. For now there is no PATH variable support.\n"
                                "In order to run any scripts/executables use: \n"
                                "\t path/to/exec [parameters]*\n\n"
                                "\tTemporarily available gsh builtin commands:\n"
                                "\t  (h)elp - prints this help screen\n"
                                "\t  (k)ill - kills proc previously set up by kernel with pid=1 (diode)\n"
                                "\t(m)orcik - prints a colorful message\n"
                                "\t  (e)cho - writes to specified file (type echo --h to get more info)\n"
                                "\t      cd - change current directory\n"
                                "\t     pwd - print working directory\n\n"
                        );
                }
                else if (strcmp(cmd, "kill") == 0 || strcmp(cmd, "k") == 0) {
                        perform_kill();
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
                else {
                        run_program(cmd);
                }
        }
}
#endif
