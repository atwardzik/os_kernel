//
// Created by Artur Twardzik on 21/08/2025.
//

#include "printer.h"
#include "mystdlib.h"
#include "escape_codes.h"
#include "kernel/memory.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"


#include <stddef.h>
#include <string.h>


extern uint8_t __screen_buffer_start__[];
extern uint8_t __screen_buffer_length__[];

uint8_t *const screen_buffer_ptr = __screen_buffer_start__;
const uint8_t *const screen_length_ptr = __screen_buffer_length__;


void raw_put_letter(const char letter, const unsigned int row_letter_position,
                    const unsigned int column_letter_position,
                    const ByteColorCode color_code
) {
        static ByteColorCode current_uart_color = (BLACK << 4) | WHITE;
        static size_t current_uart_x_position = 0;

        if (current_uart_x_position != column_letter_position) {
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

                current_uart_x_position = column_letter_position + 1;
        }


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


static inline void send_color_code(const ByteColorCode color_code) {
        const uint8_t foreground_color = color_code & FOREGROUND_COLOR_BITS;
        const uint8_t background_color = (color_code & BACKGROUND_COLOR_BITS) >> 4;

        const char *escape_str_format = "\x1b[%d;%dm";

        size_t escape_str_length = sizeof(char) * strlen(escape_str_format) + sizeof(char) * 5 + 1;
        char *escape_str = (char *) kmalloc(escape_str_length);

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

struct SingleChar {
        uint8_t ascii_code;
        ByteColorCode color_code;
};

struct CharBuffer {
        struct SingleChar chars[BUFFER_HEIGHT][BUFFER_WIDTH];
};

static struct {
        size_t current_row_position;
        size_t current_column_position;
        ByteColorCode current_color_code;
        struct CharBuffer *buffer;

        void (*put_encoded_color_letter)(char, unsigned int, unsigned int, ByteColorCode);
} ScreenWriter = {0, 0, (BLACK << 4 | WHITE), (struct CharBuffer *) screen_buffer_ptr, nullptr};

static void move_position_left() {
        if (ScreenWriter.current_column_position == 0) {
                ScreenWriter.current_row_position -= 1;
                ScreenWriter.current_column_position = BUFFER_WIDTH - 1;
        }
        else {
                ScreenWriter.current_column_position -= 1;
        }
}

static void move_position_right() {
        if (ScreenWriter.current_column_position == BUFFER_WIDTH - 1) {
                ScreenWriter.current_row_position += 1;
                ScreenWriter.current_column_position = 0;
        }
        else {
                ScreenWriter.current_column_position += 1;
        }
}

static void save_char_to_buffer(const char c) {
        const struct SingleChar ch = {c, ScreenWriter.current_color_code};

        const auto row = ScreenWriter.current_row_position;
        const auto col = ScreenWriter.current_column_position;

        ScreenWriter.buffer->chars[row][col] = ch;
}

static void scroll_vertical() {
        vga_clr_all();

        for (size_t i = 1; i < BUFFER_HEIGHT; ++i) {
                for (size_t j = 0; j < BUFFER_WIDTH; ++j) {
                        ScreenWriter.buffer->chars[i - 1][j] = ScreenWriter.buffer->chars[i][j];
                        vga_put_byte_encoded_color_letter(ScreenWriter.buffer->chars[i - 1][j].ascii_code, i - 1, j,
                                                          ScreenWriter.buffer->chars[i - 1][j].color_code);
                }
        }

        const struct SingleChar empty_char = {EMPTY_SPACE, ScreenWriter.current_color_code};

        for (size_t i = 0; i < BUFFER_WIDTH; ++i) {
                ScreenWriter.buffer->chars[BUFFER_HEIGHT - 1][i] = empty_char;
        }
}

static void scroll_horizontal(unsigned int row_position, unsigned int column_position) {
        const auto saved_row_position = row_position;
        const auto saved_column_position = column_position;

        struct SingleChar current_char = {EMPTY_SPACE, ScreenWriter.current_color_code};
        struct SingleChar next_char = ScreenWriter.buffer->chars[row_position][column_position];
        ScreenWriter.buffer->chars[row_position][column_position] = current_char;

        if (column_position == BUFFER_WIDTH - 1) {
                row_position += 1;
                column_position = 0;
        }
        else {
                column_position += 1;
        }

        while (row_position <= BUFFER_HEIGHT - 1 &&
               column_position <= BUFFER_WIDTH - 1) {
                current_char = next_char;
                next_char = ScreenWriter.buffer->chars[row_position][column_position];
                ScreenWriter.buffer->chars[row_position][column_position] = current_char;
                raw_put_letter(current_char.ascii_code, row_position, column_position, current_char.color_code);

                if (next_char.ascii_code == 0) {
                        break;
                }

                if (column_position == BUFFER_WIDTH - 1) {
                        row_position += 1;
                        column_position = 0;
                }
                else {
                        column_position += 1;
                }
        }
}

static void write_new_line() {
        save_char_to_buffer(ENDL);

        if (ScreenWriter.current_row_position == BUFFER_HEIGHT - 1) {
                scroll_vertical();
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


void write_byte(const int c) {
        static uint8_t escape_sequence[10] = {};
        static size_t escape_sequence_position = 0;

        if (escape_sequence_position || c == ESC) {
                if (c == 'm') {
                        if (escape_sequence[2] == '0' && escape_sequence_position == 3) {
                                set_background_color(&ScreenWriter.current_color_code, BLACK);
                                set_foreground_color(&ScreenWriter.current_color_code, WHITE);
                                escape_sequence_position = 0;

                                return;
                        }

                        ScreenWriter.current_color_code = decode_escape_colors(escape_sequence);
                        escape_sequence_position = 0;

                        return;
                }

                escape_sequence[escape_sequence_position] = (char) c;
                escape_sequence_position += 1;

                return;
        }


        if (c == ENDL) {
                write_new_line();
        }
        else if (c == BACKSPACE) {
                const auto row = ScreenWriter.current_row_position;
                const auto column = ScreenWriter.current_column_position;
                move_position_left();
                ScreenWriter.buffer->chars[row][column].ascii_code = 0;
        }
        else if (c == ARROW_LEFT) {
                move_position_left();
        }
        else if (c == ARROW_RIGHT) {
                move_position_right();
        }
        else {
                write_with_line_overflow_if_needed(c);
        }
}

void insert_byte(const int c) {
        const auto row = ScreenWriter.current_row_position;
        const auto column = ScreenWriter.current_column_position;

        scroll_horizontal(row, column);

        write_byte(c);
}

void write_string(const char *str) {
        const char *c = str;

        while (*c != EOL) {
                write_byte(*c);
                c += 1;
        }
}

int read_byte_with_cursor() {
        vga_setup_cursor(ScreenWriter.current_row_position, ScreenWriter.current_column_position,
                         ScreenWriter.current_color_code, 500'000);

        const int c = keyboard_receive_char();

        vga_clr_cursor();
        vga_set_cursor_off();

        return c;
}
