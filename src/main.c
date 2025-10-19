#include "tty.h"
#include "drivers/divider.h"
#include "drivers/gpio.h"
#include "drivers/keyboard.h"
#include "drivers/pio.h"
#include "drivers/time.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "fs/file.h"
#include "kernel/memory.h"
#include "kernel/proc.h"
#include "kernel/resets.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void proc0(void) {
        printf("\n\x1b[92;40m[!]Welcome from proc0\x1b[0m\n");

        int i = 0;
        while (1) {
                printf("proc0: i = %i\n", i++);
                delay_ms(1000);
        }
}

void proc1(void) {
        printf("\n\x1b[96;40m[!]Welcome from proc1\x1b[0m\n");

        while (1) {
                xor_pin(11);
                delay_ms(250);
        }
}

void proc2(void) {
        printf("\x1b[33;40m[!]Welcome from proc2\x1b[0m\n");

        char buffer[255];
        while (1) {
                printf(" > ");
                gets(buffer);
                printf("\nResponse: %s\n", buffer);
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
        scheduler_init();

        setbuf(stdout, NULL);


        // FILE const *fp = fopen("test.txt", "r");

        printf("\x1b[93;45mWelcome ");
        printf("\x1b[40;105mstring");
        printf("\x1b[0m\n");

        char buffer[255];

        while (1) {
                printf(" > ");
                // gets(buffer);
                fgets(buffer, 255, stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                if (strcmp(buffer, "r") == 0) {
                        printf("\n");

                        create_process(proc0);
                        create_process(proc1);
                        create_process(proc2);
                        run_all_processes();
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
