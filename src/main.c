#include "../include/drivers/gpio.h"
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

struct cpio_newc_header {
        char c_magic[6];
        char c_ino[8];
        char c_mode[8];
        char c_uid[8];
        char c_gid[8];
        char c_nlink[8];
        char c_mtime[8];
        char c_filesize[8];
        char c_devmajor[8];
        char c_devminor[8];
        char c_rdevmajor[8];
        char c_rdevminor[8];
        char c_namesize[8];
        char c_check[8];
};

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

void PATER_ADAMVS_SIGINT(int signum) {
        printf("\x1b[91;40mTrying to exit the init process is a bloody bad idea.\x1b[0m\n");
}

extern uint8_t __cpio_init_start__[];

void PATER_ADAMVS(int argc, char *argv[]) {
        signal(SIGINT, PATER_ADAMVS_SIGINT);
        printf("\n\x1b[96;40mPATER ADAMVS QUI EST IN PARADISO VOLVPTATIS SALVTAT SEQUENTES PROCESS FILIOS\x1b[0m\n");

        printf("\x1b[96;40m[!] Running process LED\x1b[0m\n");
        [[maybe_unused]] const int proc1_pid = spawnp(proc1, nullptr, nullptr, nullptr, nullptr);

        printf("\x1b[96;40m[!] Unpacking initramfs\x1b[0m\n");
        char *ptr = __cpio_init_start__;
        while (1) {
                const struct cpio_newc_header *header = (struct cpio_newc_header *) ptr;

                if (memcmp(header->c_magic, "070701", 6) != 0) {
                        printf("Error parsing cpio header.\n");
                        break;
                }

                if (memcmp(ptr + sizeof(*header), "TRAILER!!!", 10) == 0) {
                        printf("\x1b[96;40m[!] Unpacking ended successfully.\x1b[0m\n");
                        break;
                }

                char buf[128] = {};
                buf[8] = 0;
                buf[127] = 0;

                memcpy(buf, header->c_mode, 8);
                const auto c_mode = strtoul(buf, nullptr, 16);
                memcpy(buf, header->c_namesize, 8);
                const auto c_namesize = strtoul(buf, nullptr, 16);
                memcpy(buf, header->c_filesize, 8);
                const auto c_filesize = strtoul(buf, nullptr, 16);

                ptr += sizeof(*header);
                if (c_namesize > 128) {
                        printf("Path too long, currently unsupported.\n");
                        break;
                }
                memcpy(buf, ptr, c_namesize);

                if ((c_mode & 0xf000) == 0x4000) {
                        const int dirfd = open(buf, O_DIRECTORY | O_CREAT);
                        close(dirfd);

                        ptr += c_namesize;

                        const unsigned int padding = 4 - ((unsigned int) ptr % 4);
                        ptr += padding % 4;
                }
                else {
                        const int fd = open(buf, O_CREAT);
                        ptr += c_namesize;
                        unsigned int padding = 4 - ((unsigned int) ptr % 4);
                        ptr += padding % 4;

                        write(fd, ptr, c_filesize);

                        close(fd);

                        ptr += c_filesize;

                        padding = 4 - ((unsigned int) ptr % 4);
                        ptr += padding % 4;
                }
        }

        printf("\x1b[96;40m[!] Mounting initramfs\x1b[0m\n");
        const int cd_code = chdir("initramfs");
        if (cd_code == -1) {
                printf("\x1b[91;40m[!] No such file.\x1b[0m\n");
                __asm__("bkpt   #0");
        }

        while (1) {
                printf("\x1b[96;40m[!] Running shell (gsh)\x1b[0m\n");
                int fd = open("bin/gsh", O_BINARY);
                if (fd < 0) {
                        printf("\x1b[91;40m[!] No shell found.\x1b[0m\n");
                        __asm__("bkpt   #0");
                }

                char *const program_args[] = {"gsh", nullptr};
                [[maybe_unused]] pid_t shell_pid = spawn(fd, nullptr, nullptr, program_args, nullptr);

        process_wait:
                int code;
                const int returned_pid = wait(&code);

                if (returned_pid != shell_pid) {
                        goto process_wait; //implement waitpid syscall
                }
                printf("\n\x1b[96;40m[PATER ADAMVS]\x1b[0m Child process %i exited with code: %i\n",
                       returned_pid, code);
                close(fd);
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
        constexpr size_t root_dirs_count = 1;
        const char *root_dirs[root_dirs_count] = {"dev"};
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

        create_process_init((void (*)(void)) PATER_ADAMVS, root->inode);
        run_process_init();
        return 0;
}
