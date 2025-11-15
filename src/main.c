#include "drivers/gpio.h"
#include "drivers/keyboard.h"
#include "drivers/time.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "fs/file.h"
#include "fs/ramfs.h"
#include "kernel/memory.h"
#include "kernel/proc.h"
#include "kernel/resets.h"
#include "kernel/syscalls.h"
#include "klibc/kstdio.h"
#include "klibc/kstring.h"
#include "tty.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/wait.h>

void proc1_terminate_signal_handler(int signum) {
        if (signum == SIGTERM) {
                printf("[SIGTERM DETECTED] I don't want to exit, but as you wish\n");
        }

        exit(-1);
}

void proc1(void) {
        signal(SIGTERM, proc1_terminate_signal_handler);

        while (1) {
                xor_pin(11);
                delay_ms(250);
        }
}

int proc2_main(void) {
        printf("\x1b[33;40m[!]Welcome from proc2\x1b[0m\n");

        char buffer[255];

        printf(" ~ ");
        fgets(buffer, 255, stdin);
        printf("Response: %s\n", buffer);

        return 0;
}

void PATER_ADAMVS_SIGINT(int signum) {
        printf("\x1b[91;40mTrying to exit the init process is a bloody bad idea.\x1b[0m\n");
}

static uint16_t raw_bytes_function[] __attribute__((aligned(4))) = {
        0xf04f, 0x0001, 0xf20f, 0x010c, 0xf04f, 0x020d, 0xdf04, 0xf04f, 0x0000,
        0xdf01, 0x6548, 0x6c6c, 0x206f, 0x6f57, 0x6c72, 0x2164, 0x0000, 0xbf00,
};

static uint16_t raw_ls[] __attribute__((aligned(4))) = {
        0xb580, 0xb08e, 0xaf00, 0x6078, 0x6039, 0x683b, 0x685b, 0xf44f, 0x1100, 0x4618, 0xdf05, 0x6378, 0xe012, 0xf107,
        0x030c, 0x3308, 0xb408, 0x4618, 0xf000, 0xf824, 0x4602, 0xf04f, 0x0001, 0xbc02, 0xdf04, 0xf04f, 0x0001, 0xf20f,
        0x014a, 0xf04f, 0x0201, 0xdf04, 0xf107, 0x030c, 0x4619, 0x6b78, 0xdf08, 0x4603, 0x2b00, 0xd1e4, 0x2300, 0x4618,
        0x3738, 0x46bd, 0xf04f, 0x0000, 0xe8bd, 0x4080, 0xdf01, 0xbf00, 0xf3af, 0x8000, 0xf3af, 0x8000, 0xf3af, 0x8000,
        0x2100, 0x6802, 0x2a00, 0xd002, 0x3101, 0x3001, 0xe7f9, 0x4608, 0x4770, 0x000a,
};

void PATER_ADAMVS(int argc, char *argv[]) {
        signal(SIGINT, PATER_ADAMVS_SIGINT);
        printf("\n\x1b[96;40mPATER ADAMVS QUI EST IN PARADISO VOLVPTATIS SALVTAT SEQUENTES PROCESS FILIOS\x1b[0m\n");

        const int proc1_pid = spawn(proc1, nullptr, nullptr, nullptr, nullptr);


        while (1) {
                printf(" > ");
                char buffer[256];
                if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
                        continue;
                }
                buffer[strcspn(buffer, "\n")] = '\0';
                const char *cmd = kstrtok(buffer, " ");

                if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
                        printf("Temporarily available commands:\n"
                                "\t  (h)elp - prints this help screen\n"
                                "\t   (r)un - runs specified app, if no argument is provided runs proc2\n"
                                "\t           (standard input test). Proc1 is already running (diode)\n"
                                "\t  (k)ill - kills proc1 (diode)\n"
                                "\t(m)orcik - prints a colorful message\n"
                                "\t     rls - runs raw-bytes written ls command (mock for userspace program)\n\t---\n"
                                "\t      ls - lists current directory\n"
                                "\t   mkdir - creates a directory under specified path\n"
                                "\t   touch - creates a file under specified path\n"
                                "\t     cat - reads file contents\n"
                                "\t  (e)cho - writes to standard input or redirects to a file\n"
                                "\t rawecho - writes converted hexadecimal bytes to specified file (max 128 bytes)\n\n");
                }
                else if (strcmp(cmd, "run") == 0 || strcmp(cmd, "r") == 0) {
                        const char *path = kstrtok(nullptr, " ");
                        printf("\x1b[96;40m[PATER ADAMVS]\x1b[0m I will be waiting until my child is dead . . .\n");

                        int fd = open(path, O_BINARY);
                        if (fd > 0) {
                                uint16_t raw_bytes_app[128];
                                read(fd, raw_bytes_app, 128);
                                spawn((void *) raw_bytes_app + 1, nullptr, nullptr, nullptr, nullptr);
                        }
                        else {
                                spawn((void *) &proc2_main, nullptr, nullptr, nullptr, nullptr);
                        }

                        int code;
                        const int returned_pid = wait(&code);

                        printf("\n\x1b[96;40m[PATER ADAMVS]\x1b[0m Child process %i exited with code %i.\n",
                               returned_pid, code);
                }
                else if (strcmp(cmd, "kill") == 0 || strcmp(cmd, "k") == 0) {
                        printf("You are willing to kill the process. Choose (1) SIGTERM or (2) SIGKILL: ");

                        char line[80];
                        fgets(line, sizeof(line), stdin);
                        const int choice = strtol(line, nullptr, 10);


                        if (choice == 1) {
                                kill(proc1_pid, SIGTERM);
                        }
                        else if (choice == 2) {
                                kill(proc1_pid, SIGKILL);
                        }
                        else {
                                printf("This is not a valid signal. I won't kill the process.\n");
                        }
                }
                else if (strcmp(cmd, "morcik") == 0 || strcmp(cmd, "m") == 0) {
                        printf("\x1b[95;40mMeine beliebte Olga ist die sch\xf6nste Frau auf der Welt\n\x1b[0m");
                }
                else if (strcmp(cmd, "rls") == 0) {
                        char const *path_tok = kstrtok(nullptr, " ");
                        const char *path = path_tok ? path_tok : "";

                        char *const ls_argv[] = {"ls", path, nullptr};
                        spawn((void *) raw_ls + 1, nullptr, nullptr, ls_argv, nullptr);
                        int code;
                        const int returned_pid = wait(&code);

                        printf("\n\x1b[96;40m[PATER ADAMVS]\x1b[0m Child raw-bytes process %i exited with code %i.\n",
                               returned_pid, code);
                }
                else if (strcmp(cmd, "echo") == 0 || strcmp(cmd, "e") == 0) {
                        const char *text = kstrtok(nullptr, ">");
                        const char *path = kstrtok(nullptr, "");
                        if (!text) {
                                continue;
                        }

                        if (!path) {
                                printf("%s\n", text);
                                continue;
                        }

                        int fd = open(path, O_WRONLY);
                        if (fd < 0) {
                                printf("No such file.\n");
                                continue;
                        }

                        write(fd, text, strlen(text) + 1); // with EOF

                        close(fd);
                }
                else if (strcmp(cmd, "rawecho") == 0) {
                        const char *text = kstrtok(nullptr, ">");
                        const char *path = kstrtok(nullptr, "");
                        if (!text || !path) {
                                continue;
                        }

                        int fd = open(path, O_BINARY);
                        if (fd < 0) {
                                printf("No such file.\n");
                                continue;
                        }

                        uint8_t bytes[128] = {};
                        memset(bytes, 0, 128);
                        size_t index = 0;

                        const char *ptr = text;
                        char *endptr;
                        while (*ptr) {
                                while (*ptr == ' ') {
                                        ptr += 1;
                                }

                                if (*ptr == '\0') {
                                        break;
                                }

                                long value = strtol(ptr, &endptr, 16);

                                if (ptr == endptr) {
                                        // No valid number found
                                        break;
                                }

                                // Clamp to uint8_t range
                                if (value < 0 || value > 0xFF) {
                                        printf("Warning: value out of range (0x%lX)\n", value);
                                        break;
                                }

                                bytes[index++] = (uint8_t) value;

                                // Move pointer to next potential number
                                ptr = endptr;
                        }

                        write(fd, bytes, index);

                        close(fd);
                }
                else if (strcmp(cmd, "ls") == 0) {
                        const char *path_tok = kstrtok(nullptr, " ");
                        const char *path = path_tok == nullptr ? "" : path_tok;
                        const int dirfd = open(path, O_DIRECTORY | O_RDONLY);

                        if (dirfd < 0) {
                                printf("No such file.");
                                continue;
                        }

                        struct DirectoryEntry dentry;
                        while (readdir(dirfd, &dentry) == 1) {
                                printf("%c %s\n", dentry.file_type, dentry.name);
                        }

                        lseek(dirfd, 0, SEEK_SET);
                }
                else if (strcmp(cmd, "mkdir") == 0) {
                        const char *path = kstrtok(nullptr, " ");
                        if (!path) {
                                continue;
                        }

                        int dirfd = open(path, O_DIRECTORY | O_CREAT);
                        close(dirfd);
                }
                else if (strcmp(cmd, "touch") == 0) {
                        const char *path = kstrtok(nullptr, " ");
                        if (!path) {
                                continue;
                        }

                        int fd = open(path, O_CREAT);
                        close(fd);
                }
                else if (strcmp(cmd, "cat") == 0) {
                        const char *path = kstrtok(nullptr, " ");
                        if (!path) {
                                continue;
                        }

                        int fd = open(path, O_RDONLY);
                        if (fd < 0) {
                                printf("No such file.\n");
                                continue;
                        }

                        char file_contents[128] = {};
                        read(fd, &file_contents, 128);
                        printf("File contents: %s\n", file_contents);
                        close(fd);
                }
                else if (strcmp(cmd, "cd") == 0) {
                        const char *path = kstrtok(nullptr, " ");

                        const int code = chdir(path);
                        if (code == -1) {
                                printf("No such file.\n");
                        }
                }
                else if (strcmp(cmd, "pwd") == 0) {
                        char buf[64];
                        char *ptr = buf;
                        char *ret = getcwd(ptr, 64);

                        printf("%s\n", ret);
                }
                else {
                        printf("\x1b[96;40m[PATER ADAMVS]\x1b[0m command unknown, type (h)elp to get help.\n");
                }
        }
}

int main(void) {
        reset_subsys();
        setup_internal_clk();
        uart_init();
        uart_clr_screen();
        vga_init(8, 9, 2);

        init_pin_output(25);
        init_pin_output(11);

        void *msp;
        __asm__("mrs    %0, msp" : "=r"(msp));
        scheduler_init(msp);


        struct Dentry *root = ramfs_mount(nullptr, nullptr, nullptr, 0);
        constexpr size_t root_dirs_count = 7;
        const char *root_dirs[root_dirs_count] = {"bin", "boot", "dev", "etc", "home", "proc", "sbin"};
        for (size_t i = 0; i < root_dirs_count; ++i) {
                struct Dentry file = {
                        .name = root_dirs[i],
                };
                root->inode->i_op->create(root->inode, &file, S_IFDIR | 0666);
        }

        struct Dentry dev_dentry = {.name = "dev"};
        struct Dentry *dev = root->inode->i_op->lookup(root->inode, &dev_dentry, 0);
        struct Dentry tty_dentry = {
                .name = "tty0",
        };
        dev->inode->i_op->create(dev->inode, &tty_dentry, S_IFCHR | 0666);
        struct Dentry *tty = dev->inode->i_op->lookup(dev->inode, &tty_dentry, 0);

        init_tty();
        setup_tty_chrfile(tty->inode);
        init_keyboard(14);

        //TODO: REPLACE WITH PRINTK
        // printf("\x1b[40;47mWelcome in the kernel.\x1b[0m\n"
        //         "\x1b[92;40mSwitching to init process (temporary shell).\x1b[0m\n");
#if 0
        // create root directory and two test files

        struct Dentry file1 = {
                .name = "test.txt",
        };
        struct Dentry file2 = {
                .name = "a.out",
        };
        root_inode->i_op->create(root_inode, &file1, 0666);
        root_inode->i_op->create(root_inode, &file2, 0777);


        // usage read first file in directory
        struct File parent_handler = {
                .f_inode = root_inode,
                .f_op = root_inode->i_fop,
        };

        char buf[512];
        parent_handler.f_op->read(&parent_handler, &buf, 512, 0);
        size_t first_size = ((struct DirectoryEntry *) (&buf))->rec_len;
        struct DirectoryEntry *first_dentry = kmalloc(first_size);
        for (size_t i = 0; i < first_size; ++i) {
                *((char *) (first_dentry) + i) = buf[i];
        }

        auto first_inode = (struct VFS_Inode *) root->sb->inode_table[first_dentry->inode_index];
        printf("Mode: %o UID: %i GID: %i <time> %ldB %s\n", first_inode->i_mode,
               first_inode->i_uid,
               first_inode->i_gid, first_inode->i_size,
               first_dentry->name);

        // write to first file
        printf("\nWriting to file . . .\n\n");
        struct File first_file = {
                .f_inode = first_inode,
                .f_pos = 0,
        };
        char message[] = "Hello World!";
        first_inode->i_fop->write(&first_file, &message, sizeof(message), 0);

        char bytes_in_file[512];
        first_inode->i_fop->read(&first_file, &bytes_in_file, first_inode->i_size, 0);
        printf("New File Contents:\n\t%s\n", bytes_in_file);

        printf("New File Status:\nMode: %o UID: %i GID: %i <time> %ldB %s\n", first_inode->i_mode,
               first_inode->i_uid,
               first_inode->i_gid, first_inode->i_size,
               first_dentry->name);
#endif

        create_process_init((void (*)(void)) PATER_ADAMVS, root->inode);
        run_process_init();
        return 0;
}
