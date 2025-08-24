//
// Created by Artur Twardzik on 21/08/2025.
//

#include "stdio.h"
#include "drivers/uart.h"
#include "drivers/vga.h"


#include <stddef.h>

extern uint8_t __screen_buffer_start[];
extern uint8_t __screen_buffer_length[];

uint8_t *const screen_buffer_ptr = __screen_buffer_start;
const uint8_t *const screen_length_ptr = __screen_buffer_length;


extern char keyboard_receive_char(void);



extern void uart_putc(int c);

enum Color {
        BLACK,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
};

typedef uint8_t ByteColorCode;

struct SingleChar {
        uint8_t ascii_code;
        ByteColorCode color_code;
};


struct CharBuffer {
        struct SingleChar *chars;
};

static ByteColorCode current_color_code = GREEN << 4 | BLACK;


void putc(const int c) {
        uart_putc(c);
        vga_putc(c);
}

int getc() {
        vga_set_cursor_blink(500'000);
        const int c = keyboard_receive_char();
        vga_clr_cursor();
        vga_set_cursor_off();
        return c;
}
