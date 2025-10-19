//
// Created by Artur Twardzik on 21/08/2025.
//

#include "tty.h"

#include "escape_codes.h"
#include "myctype.h"
#include "resources.h"
#include "drivers/keyboard.h"
#include "drivers/uart.h"
#include "drivers/vga.h"

#include <stddef.h>
#include <string.h>

//TODO: probably UART should be separated from screen terminal!


extern uint8_t __screen_buffer_start__[];
extern uint8_t __screen_buffer_length__[];

uint8_t *const screen_buffer_ptr = __screen_buffer_start__;
const uint8_t *const screen_length_ptr = __screen_buffer_length__;


void raw_put_letter(
        const char letter, const unsigned int row_letter_position,
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

//TODO: tty should be dynamic kernel "process", so that we can open multiple files
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

        //TODO: check?
        // vga_update_cursor_position(ScreenWriter.current_row_position, ScreenWriter.current_column_position);
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
        // const pid_t parent_process = scheduler_get_current_process();
        // const int c = *(int *) (block_resource_on_condition(parent_process, IO_KEYBOARD, TODO));
        const int c = 65;

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


static struct {
        size_t length;
        char buffer[] __attribute__((counted_by(length)));
} *keyboard_device_file_stream;

static int keyboard_buffer_final_length = 0;
static int keyboard_buffer_current_position = 0;

static bool signal_buffer_newline = false;

void setup_keyboard_device_file() {
        constexpr size_t buf_size = 1024;

        keyboard_device_file_stream = kmalloc(
                sizeof(*keyboard_device_file_stream)
                + (buf_size - 1) * sizeof(char)
        );

        keyboard_device_file_stream->length = buf_size;
}

void *get_current_keyboard_buffer_offset() {
        return keyboard_device_file_stream->buffer + keyboard_buffer_final_length;
}

static void insert_and_shift(const char c, const int pos_insert, const int len) {
        int temp = pos_insert;
        char next_char = *(keyboard_device_file_stream->buffer + temp);
        *(keyboard_device_file_stream->buffer + temp) = c;

        temp += 1;
        while (temp < len) {
                const char current_char = next_char;
                next_char = *(keyboard_device_file_stream->buffer + temp);
                *(keyboard_device_file_stream->buffer + temp) = current_char;

                temp += 1;
        }
}

static void delete_and_shift(const int pos_delete, const int len) {
        int temp = pos_delete;

        while (temp < len) {
                *(keyboard_device_file_stream->buffer + temp) = *(keyboard_device_file_stream->buffer + temp + 1);

                temp += 1;
        }
}

void write_to_keyboard_buffer(const int c) {
        if (c == BACKSPACE) {
                if (keyboard_buffer_current_position) {
                        keyboard_buffer_final_length -= 1;
                        keyboard_buffer_current_position -= 1;
                        delete_and_shift(keyboard_buffer_current_position, keyboard_buffer_final_length);

                        write_byte(c);
                }
        }
        else if (c == ARROW_LEFT) {
                if (keyboard_buffer_current_position) {
                        keyboard_buffer_current_position -= 1;
                        write_byte(c);
                }
        }
        else if (c == ARROW_RIGHT) {
                if (keyboard_buffer_current_position < keyboard_buffer_final_length) {
                        keyboard_buffer_current_position += 1;
                        write_byte(c);
                }
        }
        else {
                keyboard_buffer_final_length += 1;
                keyboard_buffer_current_position += 1;
                if (c == ENDL) { //newline buffering
                        *(keyboard_device_file_stream->buffer + keyboard_buffer_final_length - 1) = ENDL;

                        signal_buffer_newline = true;
                        return; //TODO: ASYNCHRONIOUSLY SIGNAL
                }

                if (keyboard_buffer_current_position < keyboard_buffer_final_length) {
                        insert_and_shift(c, keyboard_buffer_current_position - 1, keyboard_buffer_final_length);

                        insert_byte(c);
                }
                else {
                        *(keyboard_device_file_stream->buffer + keyboard_buffer_current_position - 1) = (char) c;
                }
                write_byte(c);
        }
}

int newline_buffered_at() {
        if (signal_buffer_newline) {
                const auto temp = keyboard_buffer_final_length;

                signal_buffer_newline = false;
                keyboard_buffer_final_length = 0;
                keyboard_buffer_current_position = 0;

                return temp;
        }

        return false;
}

//TODO: DELETE ALL THE BELOW CODE!!!!
static ssize_t tty_read(struct File *, void *buf, size_t count, off_t file_offset) {
        __asm__("bkpt #0");
        char *ptr = (char *) buf;
        void *stream_start = get_current_keyboard_buffer_offset();

        int offset = 0;
        while (offset < count && offset < file_offset) {
                *(ptr + offset) = *(char *) (stream_start + offset);

                offset += 1;
        }

        return offset;
}

static ssize_t tty_write(struct File *, void *buf, size_t count, off_t file_offset) {
        char *ptr = (char *) buf;

        for (int i = 0; i < count; i++) {
                write_byte(*ptr++);
        }

        return count;
}

struct Files create_tty_file_mock() {
        struct FileOperations *stdin_fop = kmalloc(sizeof(*stdin_fop));
        stdin_fop->read = tty_read;
        struct FileOperations *stdout_fop = kmalloc(sizeof(*stdout_fop));
        stdout_fop->write = tty_write;

        struct File *f_stdin = kmalloc(sizeof(*f_stdin));
        f_stdin->f_pos = 0;
        f_stdin->f_op = stdin_fop;

        struct File *f_stdout = kmalloc(sizeof(*f_stdout));
        f_stdout->f_pos = 0;
        f_stdout->f_op = stdout_fop;

        struct File **fdtable = (struct File **) kmalloc(sizeof(struct File *) * MAX_OPEN_FILE_DESCRIPTORS);
        fdtable[0] = f_stdin;
        fdtable[1] = f_stdout;
        fdtable[2] = f_stdout;

        const struct Files files = {3, fdtable};

        return files;
}
