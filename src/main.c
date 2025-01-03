#include "resets.h"
#include "stdio.h"
#include "drivers/gpio.h"
#include "drivers/time.h"
#include "drivers/uart.h"
// #include "proc.h"

int main(void) {
        setup_internal_clk();
        reset_subsys();
        uart_init();

        init_pin_output(25);

        puts("Welcome string\n");

        char buffer[255];
        while (1) {
                xor_pin(25);
                puts(" > ");
                gets(buffer, 255);
                puts(buffer);
                puts("\n");
        }
        return 0;
}
