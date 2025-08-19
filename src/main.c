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

const uint8_t ascii_lookup[27][8] = {
                /*'a':*/ {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},
                /*'b':*/ {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},
                /*'c':*/ {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},
                /*'d':*/ {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},
                /*'e':*/ {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},
                /*'f':*/ {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},
                /*'g':*/ {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},
                /*'h':*/ {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},
                /*'i':*/ {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
                /*'j':*/ {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},
                /*'k':*/ {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},
                /*'l':*/ {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},
                /*'m':*/ {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},
                /*'n':*/ {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},
                /*'o':*/ {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},
                /*'p':*/ {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},
                /*'q':*/ {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},
                /*'r':*/ {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},
                /*'s':*/ {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},
                /*'t':*/ {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
                /*'u':*/ {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},
                /*'v':*/ {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},
                /*'w':*/ {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},
                /*'x':*/ {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},
                /*'y':*/ {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},
                /*'z':*/ {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},
                /*' ':*/ {0, 0, 0, 0, 0, 0, 0, 0}};

void put_letter(int position, char letter) {
        letter -= 97;
        position *= 8;
        const uint8_t *letter_lookup = ascii_lookup[letter];

        void *address_begin = (void *) 0x20035000;

        for (size_t i = 0; i < 8; ++i) {
                uint8_t line = letter_lookup[i];
                for (size_t j = 0; j < 8; ++j) {
                        if (line & (1 << j)) {
                                *(uint8_t *) (address_begin + i * 640 + j + position) = 0xff;
                        }
                }
        }
}

int main(void) {
        reset_subsys();
        setup_internal_clk();
        uart_init();
        init_keyboard(15);

        init_pin_output(25);

        hsync_gen_init(13);
        vsync_gen_init(14);
        rgb_gen_init(16);

        // uint8_t colors[] = { 0x00, 0x55, 0xff };
        // for (size_t i = 0; i < 640 * 4; ++i) {
        //        *(uint8_t *) (0x20035000 + i) = 3;
        //}

        put_letter(0, 'a');
        put_letter(1, 'r');
        put_letter(2, 't');
        put_letter(3, 'u');
        put_letter(4, 'r');

        setup_vga_dma();
        __asm__("movs r0, #7"); // start all state machines in sync
        __asm__("lsls r0, r0, #8");
        __asm__("adds r0, r0, #7");
        __asm__("ldr  r1, =0x50202000"); // atomic set
        __asm__("str  r0, [r1]");

        __asm__("movs r0, #1"); // start DMA channel
        __asm__("ldr  r1, =0x50000450");
        __asm__("str  r0, [r1]");

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
                /*for (int i = 0; i < 160; ++i) {
                        sm_put(0, 2, 0xff);
                }
                for (int i = 0; i < 160; ++i) {
                        sm_put(0, 2, 0x00);
                }*/
        }
        return 0;
}
