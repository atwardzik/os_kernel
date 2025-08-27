//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef OS_STDIO_H
#define OS_STDIO_H

#include <stddef.h>
#include <stdint.h>

constexpr uint8_t EOL = 0x00;
constexpr uint8_t BACKSPACE = 0x08;
constexpr uint8_t ENDL = 0x0A;
constexpr uint8_t CARRIAGE_RETURN = 0x0D;
constexpr uint8_t ESC = 0x1b;
constexpr uint8_t EMPTY_SPACE = 0x20;

constexpr uint32_t ARROW_LEFT = 0x1b5b44;
constexpr uint32_t ARROW_RIGHT = 0x1b5b43;

void write_string(const char *str);

void write_byte(int c);

/**
 * Inserts byte at current position, while shifting the remaining characters one position to the right
 * @param c Byte to be inserted
 */
void insert_byte(int c);

int read_byte_with_cursor();

#endif //OS_STDIO_H
