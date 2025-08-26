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
// #include "kernel/syscalls.h"
// #include "stdio.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

extern unsigned int calculate_pid_hash(pid_t pid, size_t i);

extern uint8_t __data_start__[];
extern uint8_t __bss_start__[];


int main(void) {
        reset_subsys();
        setup_internal_clk();
        uart_init();
        init_keyboard(15);

        setbuf(stdout, NULL);

        init_pin_output(25);
        init_pin_output(11);

        vga_init(13, 14, 16);

        // int *i = (int *) kmalloc(sizeof(int));

        printf("\x1b[36;40mWelcome ");
        printf("\x1b[93;40mstring");
        printf("\x1b[0m\n");

        uint8_t *data_start_ptr = __data_start__;
        uint8_t *bss_start_ptr = __bss_start__;

        // for (size_t i = 0; i < 40; i += 5) {
        //         int res = 0; //calculate_pid_hash(i, 0);
        //         char str_hash[3];
        //         if (res > 10) {
        //                 str_hash[0] = hw_div(res, 10) + 0x30;
        //                 str_hash[1] = hw_mod(res, 10) + 0x30;
        //         }
        //         else {
        //                 str_hash[0] = res + 0x30;
        //         }
        //         str_hash[2] = 0;
        //         puts(str_hash);
        // }

        char buffer[255];
        while (1) {
                printf(" > ");
                // gets(buffer, 255);
                gets(buffer);
                printf("\nResponse: %s\n", buffer);
        }
        return 0;
}
