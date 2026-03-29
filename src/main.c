#include "libc.h"
#include "tty.h"
#include "drivers/gpio.h"
#include "drivers/keyboard.h"
#include "drivers/time.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "fs/file.h"
#include "fs/ramfs.h"
#include "kernel/proc.h"
#include "kernel/resets.h"
#include "drivers/spi.h"

enum EthOperation {
        READ,
        WRITE,
};

static constexpr int eth_cs = 17;
static constexpr int eth_spi_block = 0;

/**
 * Send command to specified hw register.
 * @param op READ or WRITE
 * @param offset specified hw register's offset
 * @param data array of n bytes to be sent
 * @param len length of the bytes buffer
 */
void eth_send_command(const enum EthOperation op, uint16_t offset, char *data, const size_t len) {
        for (size_t i = 0; i < len; i++) {
                clr_pin(eth_cs);
                switch (op) {
                        case READ:
                                spi_tx(eth_spi_block, 0x0f);
                                break;
                        case WRITE:
                                spi_tx(eth_spi_block, 0xf0);
                                break;
                }

                spi_tx(eth_spi_block, offset >> 8);
                spi_tx(eth_spi_block, offset & 0x00ff);

                if (op == WRITE) {
                        spi_tx(eth_spi_block, data[i]);
                }
                else {
                        data[i] = spi_rx(eth_spi_block);
                }

                offset += 1;
                set_pin(eth_cs);
                delay_ms(1);
        }
}

void setup_ethernet(void) {
        //setup peri
        GPIO_function_select(16, 1);
        GPIO_function_select(18, 1);
        init_pin_output(eth_cs); //manual CS
        GPIO_function_select(19, 1);
        output_enable_pin(16);
        output_enable_pin(18);
        output_enable_pin(19);

        const uint32_t spi_address = spi_determine_block(0);
        spi_init(spi_address, 2, 15, 0);


        //reset
        constexpr char cmd_rst[] = {0x80};
        eth_send_command(WRITE, 0x0000, cmd_rst, sizeof(cmd_rst));

        //read VERR register
        clr_pin(17);
        spi_tx(0, 0x0f);
        spi_tx(0, 0x00);
        spi_tx(0, 0x80);
        for (int i = 0; i < 30; ++i) {
                char c = spi_rx(0);
                if (c == 0x51) {
                        printf("\x1b[96;40m[!] ETH Adapter found! Reset.\x1b[0m\n");
                }
        }
        set_pin(17);
        delay_ms(1);

        //MR = 0
        constexpr char cmd_set_mr[8] = {0x00};
        eth_send_command(WRITE, 0x0000, cmd_set_mr, sizeof(cmd_set_mr));

        //IMR = 0 - mask all interrupts, polling mode
        constexpr char cmd_set_imr[8] = {0x00};
        eth_send_command(WRITE, 0x0016, cmd_set_imr, sizeof(cmd_set_imr));

        //RTR = 0 - 200ms retransmission timeout
        constexpr char cmd_set_rtr[8] = {0x07, 0xd0};
        eth_send_command(WRITE, 0x0017, cmd_set_rtr, sizeof(cmd_set_rtr));

        //RCR = 0 - 8 retries before giving up
        constexpr char cmd_set_rcr[8] = {0x08};
        eth_send_command(WRITE, 0x0019, cmd_set_rcr, sizeof(cmd_set_rcr));

        //Set MAC
        char mac[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0x01};
        eth_send_command(WRITE, 0x0009, mac, sizeof(mac));

        //Gateway
        constexpr char gw[] = {192, 168, 1, 1};
        eth_send_command(WRITE, 0x0001, gw, sizeof(gw));

        //Subnet
        constexpr char sub[] = {255, 255, 255, 0};
        eth_send_command(WRITE, 0x0005, sub, sizeof(sub));

        //IP
        constexpr char ip[] = {192, 168, 1, 100};
        eth_send_command(WRITE, 0x000f, ip, sizeof(ip));

        //sockets
        // 4KB per socket (0x02 = 2KB, 0x04 = 4KB, 0x08 = 8KB)
        for (int i = 0; i < 4; i++) {
                constexpr char cmd[] = {0x04};
                eth_send_command(WRITE, 0x001f + 0x0100 * (i + 4), cmd, sizeof(cmd)); // 4KB TX
        }
        for (int i = 0; i < 4; i++) {
                constexpr char cmd[] = {0x04};
                eth_send_command(WRITE, 0x001e + 0x0100 * (i + 4), cmd, sizeof(cmd)); // 4KB RX
        }

        // Open socket 0 in MACRAW mode
        constexpr char mode[] = {0x04};           // MACRAW
        eth_send_command(WRITE, 0x0400, mode, 1); // Sn_MR

        constexpr char open[] = {0x01};           // OPEN command
        eth_send_command(WRITE, 0x0401, open, 1); // Sn_CR
}

void read_sd_card(void) {
        GPIO_function_select(15, 1);
        GPIO_function_select(14, 1);
        init_pin_output(13); //manual CS
        GPIO_function_select(12, 1);
        output_enable_pin(15);
        output_enable_pin(14);
        output_enable_pin(12);

        uint32_t block = spi_determine_block(1);
        spi_init(block, 2, 15, 0);

        set_pin(13);
        for (int i = 0; i < 10; ++i) { spi_tx(1, 0xff); }

        clr_pin(13);
        spi_tx(1, 0xff);
        spi_tx(1, 0x40); // CMD0
        spi_tx(1, 0x00);
        spi_tx(1, 0x00);
        spi_tx(1, 0x00);
        spi_tx(1, 0x00);
        spi_tx(1, 0x95); // valid CRC for CMD0

        for (int i = 0; i < 30; ++i) {
                if (spi_rx(1) == 0x1) {
                        printf("\x1b[96;40m[!] SD Card found!\x1b[0m\n");

                        spi_tx(1, 0x7a); // CMD58 (0x40 + 58)
                        spi_tx(1, 0x00);
                        spi_tx(1, 0x00);
                        spi_tx(1, 0x00);
                        spi_tx(1, 0x00);
                        spi_tx(1, 0x75); // dummy CRC

                        while (spi_rx(1) != 0x1) {}
                        printf("\x1b[96;40m[!] Reading SD's OCR: \x1b[0m");
                        for (int j = 0; j < 4; ++j) {
                                printf("\x1b[91;40m0x%x\x1b[0m ", spi_rx(1));
                        }
                        printf("\n");

                        break;
                }
        }


        set_pin(13);
}

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
                printf("[SIGTERM DETECTED] I don't want to exit, but as you wish.\n");
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

        read_sd_card();

        setup_ethernet();

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
                        const int dirfd = open(buf, O_DIRECTORY | O_CREAT, 0);
                        close(dirfd);

                        ptr += c_namesize;

                        const unsigned int padding = 4 - ((unsigned int) ptr % 4);
                        ptr += padding % 4;
                }
                else {
                        const int fd = open(buf, O_CREAT, 0);
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
                int fd = open("bin/gsh", O_BINARY, 0);
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
        vga_init(9, 10, 3);

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
        init_keyboard(27, 26);


        //TODO: REPLACE WITH PRINTK
        // printf("\x1b[40;47mWelcome in the kernel.\x1b[0m\n"
        //         "\x1b[92;40mSwitching to init process (temporary shell).\x1b[0m\n");

        create_process_init((void (*)(void)) PATER_ADAMVS, root->inode);
        run_process_init();
        return 0;
}
