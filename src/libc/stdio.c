//
// Created by Artur Twardzik on 21/08/2025.
//

#include "stdio.h"
#include "drivers/vga.h"
#include "drivers/uart.h"

#include "ctype.h"


static void uart_putc(const char c) {
        uart_Tx(c);

        if (c == BACKSPACE) {
                uart_Tx(EMPTY_SPACE);
                uart_Tx(BACKSPACE);
        }
        else if (c == ENDL) {
                uart_Tx(CARRIAGE_RETURN);
        }
}

void screen_putc(const char c) {
        static int row_position = 0;
        static int column_position = 0;

        //TODO: replace with scroll code
        if (row_position == BUFFER_HEIGHT) {
                row_position = 0;
                column_position = 0;
        }

        if (c == BACKSPACE && column_position > 0) {
                column_position -= 1;
                vga_put_letter(EMPTY_SPACE, row_position, column_position, 0x00, 0xff);
        }
        else if (c == BACKSPACE && column_position == 0 && row_position > 0) {
                row_position -= 1;
                column_position = BUFFER_WIDTH - 1;
                vga_put_letter(EMPTY_SPACE, row_position, column_position, 0x00, 0xff);
        }
        else if (c == ENDL) {
                column_position = 0;
                row_position += 1;
        }
        else if (isprint(c)) {
                vga_put_letter(c, row_position, column_position, 0x00, 0xff);
                column_position += 1;
                if (column_position > BUFFER_WIDTH - 1) {
                        row_position += 1;
                        column_position = 0;
                }
        }
}

void putc(const char c) {
        uart_putc(c);
        screen_putc(c);
}
