//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>

constexpr char EOL = 0x00;
constexpr char ENDL = 0x0A;
constexpr char CARRIAGE_RETURN = 0x0D;
constexpr char BACKSPACE = 0x08;
constexpr char EMPTY_SPACE = 0x20;

void putc(char c);

void puts(const char *str);

void gets(char *buffer, uint32_t size);

#endif //STDIO_H
