//
// Created by Artur Twardzik on 21/08/2025.
//

#include "stdio.h"
#include "drivers/uart.h"
#include "drivers/vga.h"

#include "ctype.h"

static int screen_row_position = 0;
static int screen_column_position = 0;

extern char keyboard_receive_char(void);

extern void set_cursor_blink(uint32_t us);
extern void set_cursor_off(void);

static void uart_putc(const int c) {
        if (c > 255) {
                uint8_t byte0 = (c & 0xff0000) >> 16;
                uart_Tx(byte0);

                uint8_t byte1 = (c & 0xff00) >> 8;
                uart_Tx(byte1);

                uint8_t byte2 = c & 0xff;
                uart_Tx(byte2);

                return;
        }

        uart_Tx(c);

        if (c == BACKSPACE) {
                uart_Tx(EMPTY_SPACE);
                uart_Tx(BACKSPACE);
        }
        else if (c == ENDL) {
                uart_Tx(CARRIAGE_RETURN);
        }
}


void screen_putc(const int c) {
        if (c > 255) {
                uint8_t direction = c & 0xff;

                // TODO: add proper cursor management
                if (direction == 0x43) {
                        screen_column_position += 1;
                }
                else if (direction == 0x44) {
                        screen_column_position -= 1;
                }

                return;
        }

        // TODO: replace with scroll code
        if (screen_row_position == BUFFER_HEIGHT) {
                screen_row_position = 0;
                screen_column_position = 0;
        }

        if (c == BACKSPACE && screen_column_position > 0) {
                screen_column_position -= 1;
                vga_put_letter(EMPTY_SPACE, screen_row_position, screen_column_position, 0x00, 0xff);
        }
        else if (c == BACKSPACE && screen_column_position == 0 && screen_row_position > 0) {
                screen_row_position -= 1;
                screen_column_position = BUFFER_WIDTH - 1;
                vga_put_letter(EMPTY_SPACE, screen_row_position, screen_column_position, 0x00, 0xff);
        }
        else if (c == ENDL) {
                screen_column_position = 0;
                screen_row_position += 1;
        }
        else if (isprint(c)) {
                vga_put_letter(c, screen_row_position, screen_column_position, 0x00, 3 << 2);
                screen_column_position += 1;
                if (screen_column_position > BUFFER_WIDTH - 1) {
                        screen_row_position += 1;
                        screen_column_position = 0;
                }
        }
}

void putc(const int c) {
        uart_putc(c);
        screen_putc(c);
}

void xor_cursor() {
        static bool cursor_on = false;

        if (cursor_on) {
                vga_put_letter(CURSOR_FULL, screen_row_position, screen_column_position, 0x00, 3 << 2);
                cursor_on = false;
        }
        else {
                vga_put_letter(EMPTY_SPACE, screen_row_position, screen_column_position, 0x00, 3 << 2);
                cursor_on = true;
        }
}

void clr_cursor() { vga_put_letter(EMPTY_SPACE, screen_row_position, screen_column_position, 0x00, 3 << 2); }

int getc() {
        set_cursor_blink(500'000);
        const int c = keyboard_receive_char();
        clr_cursor();
        set_cursor_off();
        return c;
}
