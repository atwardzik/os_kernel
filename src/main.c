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
#include "kernel/syscalls.h"
#include "stdio.h"

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
        rgb_gen_init(16);

        setup_vga_dma();
        __asm__("movs r0, #7"); // start all state machines in sync
        __asm__("lsls r0, r0, #8");
        __asm__("adds r0, r0, #7");
        __asm__("ldr  r1, =0x50202000"); // atomic set
        __asm__("str  r0, [r1]");

        __asm__("movs r0, #1"); // start DMA channel
        __asm__("ldr  r1, =0x50000450");
        __asm__("str  r0, [r1]");

        int *i = (int *) kmalloc(sizeof(int));

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
                // char *ptr = buffer;
                // int i = 0;
                // while (*ptr) {
                //         put_letter(i, *ptr);
                //         ptr += 1;
                //         i += 1;
                // }
                puts("\n");
                __asm__("svc #0");
                /*for (int i = 0; i < 160; ++i) {
                        sm_put(0, 2, 0xff);
                }
                for (int i = 0; i < 160; ++i) {
                        sm_put(0, 2, 0x00);
                }*/
        }
        return 0;
}
