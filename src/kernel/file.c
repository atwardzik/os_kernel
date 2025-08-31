//
// Created by Artur Twardzik on 27/08/2025.
//

#include "file.h"
#include "proc.h"
#include "printer.h"

#include <stdio.h>

constexpr int MAX_OPEN_FILE_DESCRIPTORS = 8;

static FILE *file_descriptors[MAX_OPEN_FILE_DESCRIPTORS] = {};

static void (*file_writers[MAX_OPEN_FILE_DESCRIPTORS])(int) = {};

static int (*file_readers[MAX_OPEN_FILE_DESCRIPTORS])(void) = {};


void init_file_descriptors(void) {
        file_descriptors[0] = stdin;
        file_descriptors[1] = stdout;
        file_descriptors[2] = stderr;

        file_readers[0] = &read_byte_with_cursor;

        file_writers[1] = &write_byte;
        file_writers[2] = &write_byte;
}


int sys_open(const char *name, int flags, int mode) {
        return -1;
}

int sys_close(int file) {
        return -1;
}

static void insert_and_shift(const char c, char *ptr, const int pos_insert, const int len) {
        int temp = pos_insert;
        char next_char = *(ptr + temp);
        char current_char;
        *(ptr + temp) = c;

        temp += 1;
        while (temp < len) {
                current_char = next_char;
                next_char = *(ptr + temp);
                *(ptr + temp) = current_char;

                temp += 1;
        }
}

static void delete_and_shift(char *ptr, const int pos_delete, int len) {
        int temp = pos_delete;

        while (temp < len) {
                *(ptr + temp) = *(ptr + temp + 1);

                temp += 1;
        }
}

static int read_stdin(char *ptr, int len) {
        int final_length = 0;
        int current_position = 0;
        while (final_length < len) {
                const int c = read_byte_with_cursor(); //TODO: setup waiting for this particular interrupt
                // it must get back EXACTLY here...

                if (c == BACKSPACE) {
                        if (current_position) {
                                final_length -= 1;
                                current_position -= 1;
                                delete_and_shift(ptr, current_position, final_length);
                                write_byte(c);
                        }
                }
                else if (c == ARROW_LEFT) {
                        if (current_position) {
                                current_position -= 1;
                                write_byte(c);
                        }
                }
                else if (c == ARROW_RIGHT) {
                        if (current_position < final_length) {
                                current_position += 1;
                                write_byte(c);
                        }
                }
                else {
                        final_length += 1;
                        current_position += 1;
                        if (c == ENDL) {
                                *(ptr + final_length - 1) = ENDL;
                                break;
                        }

                        if (current_position < final_length) {
                                insert_and_shift(c, ptr, current_position - 1, final_length);

                                insert_byte(c);
                                continue;
                        }
                        else {
                                *(ptr + current_position - 1) = (char) c;
                        }
                        write_byte(c);
                }
        }

        return final_length;
}

int sys_read(int file, char *ptr, int len) {
        if (file == 0) {
                return read_stdin(ptr, len);
        }

        if (file > MAX_OPEN_FILE_DESCRIPTORS - 1 || file < 0) {
                sys_write(1, "[!] There is no such file descriptor\n", 37);
                __asm__("bkpt   #0");
                return 0;
        }

        int (*file_reader)(void) = file_readers[file];

        if (file_reader == nullptr) {
                sys_write(1, "[!] There is no such file descriptor\n", 37);
                __asm__("bkpt   #0");
                return 0;
        }

        int offset = 0;
        while (offset < len) {
                const int c = file_reader();

                *(ptr + offset) = (char) c;

                offset += 1;
        }

        return offset;
}

int sys_write(const int file, char *ptr, const int len) {
        if (file > MAX_OPEN_FILE_DESCRIPTORS - 1 || file < 0) {
                sys_write(1, "[!] There is no such file descriptor\n", 37);
                __asm__("bkpt   #0");
                return 0;
        }

        void (*file_writer)(int) = file_writers[file];

        if (file_writer == nullptr) {
                sys_write(1, "[!] There is no such file descriptor\n", 37);
                __asm__("bkpt   #0");
                return 0;
        }

        for (int i = 0; i < len; i++) {
                file_writer(*ptr++);
        }

        return len;
}
