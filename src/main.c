#include "kstdio.h"
#include "tty.h"
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

void PATER_ADAMVS(void) {
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

                if (strcmp(buffer, "help") == 0 || strcmp(buffer, "h") == 0) {
                        printf("Temporarily available commands:\n"
                                "\t  (h)elp - prints this help screen\n"
                                "\t   (r)un - runs proc2 (standard input test). Proc1 is already running (diode)\n"
                                "\t  (k)ill - kills proc1 (diode)\n"
                                "\t(m)orcik - prints a colorful message\n"
                                "\t  (o)pen - opens file file.txt\n"
                                "\t      ls - lists current directory\n\n");
                }
                else if (strcmp(buffer, "run") == 0 || strcmp(buffer, "r") == 0) {
                        printf("\x1b[96;40m[PATER ADAMVS]\x1b[0m I will be waiting until my child is dead . . .\n");

                        spawn((void *) &proc2_main, nullptr, nullptr, nullptr, nullptr);
                        int code;
                        const int returned_pid = wait(&code);

                        printf("\n\x1b[96;40m[PATER ADAMVS]\x1b[0m Child process %i exited with code %i.\n",
                               returned_pid, code);
                }
                else if (strcmp(buffer, "kill") == 0 || strcmp(buffer, "k") == 0) {
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
                else if (strcmp(buffer, "morcik") == 0 || strcmp(buffer, "m") == 0) {
                        printf("\x1b[95;40mMeine beliebte Olga ist die sch\xf6nste Frau auf der Welt\n\x1b[0m");
                }
                else if (strcmp(buffer, "open") == 0 || strcmp(buffer, "o") == 0) {
                        int dirfd1 = open("dir1", O_DIRECTORY | O_CREAT);

                        int fd = open("dir1/test.txt", O_RDWR | O_CREAT);

                        char file_contents[100];
                        read(fd, &file_contents, 100);
                        printf("File contents pre-write: %s\n", file_contents);
                        constexpr char new_file_contents[] = "HELLO WORLD [ANGRIER]!";
                        write(fd, &new_file_contents, sizeof(new_file_contents));
                        read(fd, &file_contents, 100);
                        printf("File contents post-write: %s\n", file_contents);
                }
                else if (strcmp(buffer, "rb") == 0) {
                        spawn((void *) raw_bytes_function, nullptr, nullptr, nullptr, nullptr);
                        int code;
                        const int returned_pid = wait(&code);

                        printf("\n\x1b[96;40m[PATER ADAMVS]\x1b[0m Child raw-bytes process %i exited with code %i.\n",
                               returned_pid, code);
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
        init_keyboard(14);

        init_pin_output(25);
        init_pin_output(11);

        init_tty();
        setup_keyboard_device_file();

        void *msp;
        __asm__("mrs    %0, msp" : "=r"(msp));
        scheduler_init(msp);


        printf("\x1b[40;47mWelcome in the kernel.\x1b[0m\n"
                "\x1b[92;40mSwitching to init process (temporary shell).\x1b[0m\n"
        );


        // create root directory and two test files
        const struct Dentry *root = ramfs_mount(nullptr, nullptr, nullptr, 0);
#if 0
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

        create_process_init(PATER_ADAMVS, root->inode);
        run_process_init();
        return 0;
}