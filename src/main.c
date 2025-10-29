#include "kstdio.h"
#include "tty.h"
#include "drivers/gpio.h"
#include "drivers/keyboard.h"
#include "drivers/time.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "kernel/proc.h"
#include "kernel/resets.h"
#include "kernel/syscalls.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

        printf(" > ");
        fgets(buffer, 255, stdin);
        printf("\nResponse: %s\n", buffer);

        return 0;
}

// void proc2_start(void) {
//         // setup crt
//
//         int ret_value = proc2_main();
//
//         exit(ret_value);
// }

void PATER_ADAMVS_SIGINT(int signum) {
        printf("Trying to exit the init process is a bloody bad idea.\n");
}

void PATER_ADAMVS(void) {
        signal(SIGINT, PATER_ADAMVS_SIGINT);
        printf("\n\x1b[96;40mPATER ADAMVS QUI EST IN PARADISO VOLVPTATIS SALVTAT SEQUENTES PROCESS FILIOS\x1b[0m\n");

        int proc1_pid = spawn(proc1, nullptr, nullptr, nullptr, nullptr);


        char buffer[256];
        while (1) {
                printf(" > ");
                fgets(buffer, 256, stdin);
                buffer[strcspn(buffer, "\n")] = '\0';

                if (strcmp(buffer, "help") == 0 || strcmp(buffer, "h") == 0) {
                        printf("Temporarily available commands:\n"
                                "\t  (h)elp - prints this help screen\n"
                                "\t   (r)un - runs proc2 (standard input test). Proc1 is already running (diode)\n"
                                "\t  (k)ill - kills proc1 (diode)\n"
                                "\t(m)orcik - prints a colorful message\n"
                                "\t      ls - lists current directory\n\n"
                        );
                }
                else if (strcmp(buffer, "run") == 0 || strcmp(buffer, "r") == 0) {
                        printf("[PATER ADAMVS] I will be waiting until my child is dead . . .\n");

                        spawn((void *) &proc2_main, nullptr, nullptr, nullptr, nullptr);
                        int code;
                        const int returned_pid = wait(&code);

                        printf("[PATER ADAMVS] My child with ID:%i is dead with code %i. I can now resume my exec.\n",
                               returned_pid, code
                        );
                }
                else if (strcmp(buffer, "kill") == 0 || strcmp(buffer, "k") == 0) {
                        printf("You are willing to kill the process. Choose (1) SIGTERM or (2) SIGKILL: ");

                        char line[80];
                        fgets(line, sizeof(line), stdin);
                        const int choice = strtol(line, nullptr, 10);


                        if (choice == 1) {
                                kill(proc1_pid, SIGKILL);
                        }
                        else if (choice == 2) {
                                kill(proc1_pid, SIGTERM);
                        }
                        else {
                                printf("This is not a valid signal. I won't kill the process.\n");
                        }
                }
                else if (strcmp(buffer, "morcik") == 0 || strcmp(buffer, "m") == 0) {
                        printf("\x1b[95;40mMeine beliebte Olga ist die sch\xf6nste Frau auf der Welt\n\x1b[0m");
                }
                else {
                        printf("[PATER ADAMVS] command unknown, type (h)elp to get help.\n");
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

        setbuf(stdout, NULL);

        printf("\x1b[40;47mWelcome in the kernel.\x1b[0m\n"
                "\x1b[92;40mSwitching to init process (temporary shell).\x1b[0m\n"
        );

        create_process_init(PATER_ADAMVS);
        run_process_init();

        return 0;
}
