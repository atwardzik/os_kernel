#include "drivers/divider.h"
#include "drivers/gpio.h"
#include "drivers/keyboard.h"
#include "drivers/pio.h"
#include "drivers/time.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "kernel/memory.h"
#include "kernel/proc.h"
#include "kernel/resets.h"
#include "kernel/file.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void proc0(void) {
        printf("\x1b[33;40m[!]Welcome from proc0\x1b[0m\n");

        int i = 0;
        while (1) {
                printf("proc0: i = %i\n", i++);
                delay_ms(1000);
        }
}

void proc1(void) {
        init_pin_output(25);

        while (1) {
                xor_pin(25);
                delay_ms(500);
        }
}

#if 0
void proc2(void) {
        printf("\x1b[33;40m[!]Welcome from proc2\x1b[0m\n");

        char buffer[255];
        while (1) {
                printf(" > ");
                gets(buffer);
                printf("\nResponse: %s\n", buffer);
        }
}
#endif


int main(void) {
        reset_subsys();
        setup_internal_clk();
        uart_init();
        uart_clr_screen();
        init_keyboard(15);
        vga_init(13, 14, 16);

        init_pin_output(25);

        init_file_descriptors();
        scheduler_init();

        setbuf(stdout, NULL);


        // FILE const *fp = fopen("test.txt", "r");

        printf("\x1b[33;45mWelcome ");
        printf("\x1b[93;105mstring");
        printf("\x1b[0m\n");

        char buffer[255];
        printf(" > ");
        gets(buffer);
        if (strcmp(buffer, "r p") == 0) {
                create_process(proc0);
                create_process(proc1);
                run_all_processes();
        }

        while (1) {
                // printf(" $ ");
                // gets(buffer);
                // printf("\n\x1b[91;40mKernel Response:\x1b[0m %s\n", buffer);
        }
        return 0;
}
