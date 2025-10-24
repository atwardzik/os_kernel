#include "kstdio.h"
#include "tty.h"
#include "drivers/gpio.h"
#include "drivers/keyboard.h"
#include "drivers/time.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "kernel/proc.h"
#include "kernel/resets.h"

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>


void proc1(void) {
        printf("\n\x1b[96;40m[!]Welcome from proc1\x1b[0m\n");

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

void proc2_start(void) {
        // setup crt

        int ret_value = proc2_main();

        sys_exit(ret_value);
}

void PATER_ADAMVS(void) {
        printf("\n\x1b[96;40mPATER ADAMVS QUI EST IN PARADISO VOLVPTATIS SALVTAT SEQUENTES PROCESS FILIOS\x1b[0m\n");

        sys_spawn_process(proc1, nullptr, nullptr, nullptr, nullptr);
        sys_spawn_process(proc2_start, nullptr, nullptr, nullptr, nullptr);

        int i = 0;
        while (1) {
                printf("PATER ADAMVS DINUMERO: i = %i\n", i++);
                delay_ms(1000);
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


        // FILE const *fp = fopen("test.txt", "r");

        printf("\x1b[93;45mWelcome ");
        printf("\x1b[40;105mstring");
        printf("\x1b[0m\n");

        char buffer[255];

        while (1) {
                printf(" > ");
                fgets(buffer, 255, stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                if (strcmp(buffer, "r") == 0) {
                        printf("\n");

                        create_process_init(PATER_ADAMVS);
                        run_process_init();
                }
                else if (strcmp(buffer, "morcik") == 0) {
                        printf("\n\x1b[95;40mMeine beliebte Olga ist die sch\xf6nste Frau auf der Welt\n\x1b[0m");
                }
                else {
                        printf("\n\x1b[91;40mKernel Response:\x1b[0m %s\n", buffer);
                }
        }
        return 0;
}
