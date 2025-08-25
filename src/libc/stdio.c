//
// Created by Artur Twardzik on 21/08/2025.
//

#include "stdio.h"
#include "stdlib.h"
#include "escape_codes.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"


#include <stddef.h>

#include "ctype.h"

extern uint8_t __screen_buffer_start[];
extern uint8_t __screen_buffer_length[];

uint8_t *const screen_buffer_ptr = __screen_buffer_start;
const uint8_t *const screen_length_ptr = __screen_buffer_length;


void raw_putc(const int c) {
        uart_putc(c);
        vga_putc(c);
}

void raw_put_letter(const char letter, const unsigned int row_letter_position,
                    const unsigned int column_letter_position,
                    const ByteColorCode color_code
) {
        static ByteColorCode current_uart_color = (BLACK << 4) | WHITE;

        char row_position[5] = {};
        char col_position[5] = {};

        uart_putc(0x1b);
        uart_putc('[');

        itoa(row_letter_position + 1, row_position, 10);
        uart_puts(row_position);

        uart_putc(';');

        itoa(column_letter_position + 1, col_position, 10);
        uart_puts(col_position);

        uart_putc('H');

        if (current_uart_color != color_code) {
                current_uart_color = color_code;

                const uint8_t foreground_color = color_code & FOREGROUND_COLOR_BITS;
                uint8_t background_color = (color_code & BACKGROUND_COLOR_BITS) >> 4;

                if (background_color == BLACK) {
                        background_color = 9;
                }

                uart_putc(0x1b);
                uart_putc('[');
                if (foreground_color > WHITE) {
                        uart_putc('9');
                }
                else {
                        uart_putc('3');
                }
                uart_putc(foreground_color + 0x30);

                uart_putc(';');

                if (background_color > WHITE) {
                        uart_putc('1');
                        uart_putc('0');
                }
                else {
                        uart_putc('4');
                }
                uart_putc(background_color + 0x30);

                uart_putc('m');
        }

        uart_putc(letter);

        vga_put_byte_encoded_color_letter(letter, row_letter_position, column_letter_position, color_code);
}

int getc() {
        vga_set_cursor_blink(500'000);
        const int c = keyboard_receive_char();
        vga_clr_cursor();
        vga_set_cursor_off();
        return c;
}

struct SingleChar {
        uint8_t ascii_code;
        ByteColorCode color_code;
};

struct CharBuffer {
        struct SingleChar chars[BUFFER_HEIGHT][BUFFER_WIDTH];
};

static void refresh_screens(void);

static struct {
        size_t current_row_position;
        size_t current_column_position;
        ByteColorCode current_color_code;
        struct CharBuffer *buffer;

        void (*refresh)(void);
} ScreenWriter = {0, 0, (BLACK << 4 | WHITE), (struct CharBuffer *) screen_buffer_ptr, refresh_screens};

static void save_char_to_buffer(const char c) {
        const struct SingleChar ch = {c, ScreenWriter.current_color_code};

        const auto row = ScreenWriter.current_row_position;
        const auto col = ScreenWriter.current_column_position;

        ScreenWriter.buffer->chars[row][col] = ch;
}

static void scroll() {
        vga_clr_all();

        for (size_t i = 1; i < BUFFER_HEIGHT; ++i) {
                for (size_t j = 0; j < BUFFER_WIDTH; ++j) {
                        ScreenWriter.buffer->chars[i - 1][j] = ScreenWriter.buffer->chars[i][j];
                        vga_put_byte_encoded_color_letter(ScreenWriter.buffer->chars[i - 1][j].ascii_code, i - 1, j,
                                                          ScreenWriter.buffer->chars[i - 1][j].color_code);
                }
        }

        struct SingleChar empty_char = {EMPTY_SPACE, ScreenWriter.current_color_code};

        for (size_t i = 0; i < BUFFER_WIDTH; ++i) {
                ScreenWriter.buffer->chars[BUFFER_HEIGHT - 1][i] = empty_char;
        }

        // ScreenWriter.refresh();
}

static void write_new_line() {
        save_char_to_buffer(ENDL);

        if (ScreenWriter.current_row_position == BUFFER_HEIGHT - 1) {
                scroll();
        }
        else {
                ScreenWriter.current_row_position += 1;
        }

        raw_put_letter(ENDL, ScreenWriter.current_row_position,
                       ScreenWriter.current_column_position,
                       ScreenWriter.current_color_code);
        ScreenWriter.current_column_position = 0;
}

static void write_with_line_overflow_if_needed(const char c) {
        save_char_to_buffer(c);

        raw_put_letter(c, ScreenWriter.current_row_position,
                       ScreenWriter.current_column_position,
                       ScreenWriter.current_color_code);

        ScreenWriter.current_column_position += 1;
        if (ScreenWriter.current_column_position == BUFFER_WIDTH) {
                write_new_line();
        }
}

static void write_byte(const char c) {
        if (c == ENDL) {
                write_new_line();
        }
        else {
                write_with_line_overflow_if_needed(c);
        }
}

static void write_string(const char *str) {
        static uint8_t escape_sequence[10] = {};
        static size_t escape_sequence_position = 0;

        const char *c = str;

        while (*c != EOL) {
                if (escape_sequence_position || *c == ESC) {
                        if (*c == 'm') {
                                if (escape_sequence[2] == '0' && escape_sequence_position == 3) {
                                        set_background_color(&ScreenWriter.current_color_code, BLACK);
                                        set_foreground_color(&ScreenWriter.current_color_code, WHITE);
                                        escape_sequence_position = 0;

                                        c += 1;
                                        continue;
                                }

                                ScreenWriter.current_color_code = decode_escape_colors(escape_sequence);
                                escape_sequence_position = 0;

                                c += 1;
                                continue;
                        }

                        escape_sequence[escape_sequence_position] = *c;
                        escape_sequence_position += 1;
                }
                else {
                        write_byte(*c);
                }

                c += 1;
        }
}


static inline void send_color_code(const ByteColorCode color_code) {
        const uint8_t foreground_color = color_code & FOREGROUND_COLOR_BITS;
        const uint8_t background_color = color_code & BACKGROUND_COLOR_BITS;

        vga_putc(0x1b);
        vga_putc('[');
        if (foreground_color > WHITE) {
                vga_putc('9');
        }
        else {
                vga_putc('3');
        }
        vga_putc(foreground_color + 0x30);

        vga_putc(';');

        if (background_color > WHITE) {
                vga_putc('1');
                vga_putc('0');
        }
        else {
                vga_putc('4');
        }
        vga_putc(background_color + 0x30);

        vga_putc('m');
}

static void refresh_screens() {
        // uart_clr_screen();
        vga_clr_all();

        ByteColorCode current_global_color_code = ScreenWriter.current_color_code;
        for (size_t i = 0; i < BUFFER_HEIGHT; ++i) {
                for (size_t j = 0; j < BUFFER_WIDTH; ++j) {
                        const ByteColorCode current_byte_color_code = ScreenWriter.buffer->chars[i][j].color_code;
                        const uint8_t current_ascii_code = ScreenWriter.buffer->chars[i][j].ascii_code;

                        if (current_byte_color_code != current_global_color_code) {
                                send_color_code(current_byte_color_code);
                                current_global_color_code = current_byte_color_code;
                        }

                        // raw_put_letter(current_ascii_code, i, j, current_byte_color_code);
                        vga_put_byte_encoded_color_letter(current_ascii_code, i, j, current_byte_color_code);
                }
        }
}

void putc(const int c) {
        write_byte(c);
}

void printf(const char *str) {
        write_string(str);
}
