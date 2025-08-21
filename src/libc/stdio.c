//
// Created by Artur Twardzik on 21/08/2025.
//

#include "stdio.h"
#include "drivers/vga.h"
#include "drivers/uart.h"

#include "ctype.h"

static int screen_row_position = 0;
static int screen_column_position = 0;

extern char keyboard_receive_char(void);

extern void set_cursor_blink(uint32_t us);

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
        //TODO: replace with scroll code
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

void putc(const char c) {
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

void clr_cursor() {
        vga_put_letter(EMPTY_SPACE, screen_row_position, screen_column_position, 0x00, 3 << 2);
}

char getc() {
        set_cursor_blink(500'000);
        const char c = keyboard_receive_char();
        clr_cursor();
        
        return c;
}
