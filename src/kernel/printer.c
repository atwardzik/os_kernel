//
// Created by Artur Twardzik on 21/08/2025.
//

#include "printer.h"
#include "myctype.h"
#include "escape_codes.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "resources.h"


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
        uart_set_cursor(row_letter_position, column_letter_position);

        uart_change_color(color_code);

        if (isprint(letter)) {
                uart_putc(letter);
        }
        else {
                uart_putc(EMPTY_SPACE);
        }


        vga_put_byte_encoded_color_letter(letter, row_letter_position, column_letter_position, color_code);
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
} ScreenWriter = {0, 0, (BLACK << 4 | WHITE), (struct CharBuffer *) screen_buffer_ptr};

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
        uart_puts("\x1b[S");

        for (size_t i = 1; i < BUFFER_HEIGHT; ++i) {
                for (size_t j = 0; j < BUFFER_WIDTH; ++j) {
                        ScreenWriter.buffer->chars[i - 1][j] = ScreenWriter.buffer->chars[i][j];
                        vga_put_byte_encoded_color_letter(ScreenWriter.buffer->chars[i - 1][j].ascii_code, i - 1, j,
                                                          ScreenWriter.buffer->chars[i - 1][j].color_code);
                }
        }

        const struct SingleChar empty_char = {0x00, ScreenWriter.current_color_code};

        for (size_t i = 0; i < BUFFER_WIDTH; ++i) {
                ScreenWriter.buffer->chars[BUFFER_HEIGHT - 1][i] = empty_char;
        }
}

static void scroll_horizontal_right(unsigned int row_position, unsigned int column_position) {
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

static void scroll_horizontal_left(const unsigned int row_position, const unsigned int column_position) {
        for (size_t i = row_position; i < BUFFER_HEIGHT; ++i) {
                const size_t column_starting_point = (i == row_position) ? column_position : 0;

                for (size_t j = column_starting_point + 1; j <= BUFFER_WIDTH; ++j) {
                        struct SingleChar current_char;

                        if (i == BUFFER_HEIGHT - 1 && j == BUFFER_WIDTH) {
                                const struct SingleChar c = {0x00, (BLACK << 4) | WHITE};
                                current_char = c;
                        }
                        else if (j == BUFFER_WIDTH) {
                                current_char = ScreenWriter.buffer->chars[i + 1][0];
                        }
                        else {
                                current_char = ScreenWriter.buffer->chars[i][j];
                        }

                        ScreenWriter.buffer->chars[i][j - 1] = current_char;

                        raw_put_letter(current_char.ascii_code, i, j - 1, current_char.color_code);

                        if (current_char.ascii_code == 0) {
                                uart_set_cursor(row_position, column_position);
                                return;
                        }
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
                move_position_left();

                uart_putc(c);

                const auto row = ScreenWriter.current_row_position;
                const auto column = ScreenWriter.current_column_position;
                scroll_horizontal_left(row, column);
        }
        else if (c == ARROW_LEFT) {
                move_position_left();
                uart_putc(c);
        }
        else if (c == ARROW_RIGHT) {
                move_position_right();
                uart_putc(c);
        }
        else {
                write_with_line_overflow_if_needed(c);
        }
}

void insert_byte(const int c) {
        const auto row = ScreenWriter.current_row_position;
        const auto column = ScreenWriter.current_column_position;

        scroll_horizontal_right(row, column);

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
        const auto row = ScreenWriter.current_row_position;
        const auto column = ScreenWriter.current_column_position;
        vga_setup_cursor(row, column, ScreenWriter.current_color_code, 500'000);
        uart_set_cursor(row, column);

        clr_keyboard_buffer();
        const pid_t parent_process = scheduler_get_current_process();
        const int c = *(int *) (block_on_resource(parent_process, IO_KEYBOARD));

        vga_clr_cursor();
        vga_set_cursor_off();

        return c;
}

int kread_byte_with_cursor(void) {
        const auto row = ScreenWriter.current_row_position;
        const auto column = ScreenWriter.current_column_position;
        vga_setup_cursor(row, column, ScreenWriter.current_color_code, 500'000);
        uart_set_cursor(row, column);

        const int c = keyboard_receive_char(); //spinlock

        vga_clr_cursor();
        vga_set_cursor_off();

        return c;
}
