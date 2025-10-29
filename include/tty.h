//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef OS_STDIO_H
#define OS_STDIO_H

#include "stdint.h"
#include "types.h"
#include "fs/file.h"

constexpr uint8_t EOL = 0x00;
constexpr uint8_t ETX = 0x03;
constexpr uint8_t EOT = 0x04;

constexpr uint8_t BEL = 0x07;
constexpr uint8_t BACKSPACE = 0x08;
constexpr uint8_t HT = 0x09;
constexpr uint8_t ENDL = 0x0A;
constexpr uint8_t VT = 0x0B;
constexpr uint8_t FF = 0x0C;
constexpr uint8_t CARRIAGE_RETURN = 0x0D;
constexpr uint8_t ESC = 0x1b;
constexpr uint8_t EMPTY_SPACE = 0x20;

constexpr uint32_t ARROW_LEFT = 0x1b5b44;
constexpr uint32_t ARROW_RIGHT = 0x1b5b43;

void init_tty(void);

void write_byte(const int c);

void setup_keyboard_device_file(void);

void write_to_keyboard_buffer(int c);

void *get_current_keyboard_buffer_offset(void);

int newline_buffered_at(void);

ssize_t tty_read(struct File *, void *buf, const size_t count, off_t file_offset);

ssize_t tty_write(struct File *, void *buf, const size_t count, off_t file_offset);

[[deprecated]]
struct Files create_tty_file_mock(void);

#endif //OS_STDIO_H
