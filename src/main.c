#include "divider.h"
#include "drivers/gpio.h"
#include "drivers/keyboard.h"
#include "drivers/pio.h"
#include "drivers/time.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "memory.h"
#include "proc.h"
#include "resets.h"
#include "stdio.h"
#include "syscalls.h"

#include <stddef.h>
#include <stdint.h>

extern unsigned int calculate_pid_hash(pid_t pid, size_t i);

extern uint8_t __data_start[];
extern uint8_t __bss_start[];

int main(void) {
        reset_subsys();
        setup_internal_clk();
        uart_init();
        init_keyboard(15);

        init_pin_output(25);

        hsync_gen_init(13);
        vsync_gen_init(14);

        // int *i = (int *) kmalloc(sizeof(int));

        puts("Welcome string\n");
        uint8_t *data_start_ptr = __data_start;
        uint8_t *bss_start_ptr = __bss_start;

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

        load_pio_prog(1, 2, 3);

        char buffer[255];
        while (1) {
                xor_pin(25);
                puts(" > ");
                gets(buffer, 255);
                puts(buffer);
                puts("\n");
                __asm__("svc #0");
        }
        return 0;
}
