//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>

constexpr uint8_t EOL = 0x00;
constexpr uint8_t BACKSPACE = 0x08;
constexpr uint8_t ENDL = 0x0A;
constexpr uint8_t CARRIAGE_RETURN = 0x0D;
constexpr uint8_t ESC = 0x1b;
constexpr uint8_t EMPTY_SPACE = 0x20;

/**
 * Writes character at the current screen position.
 *
 * @param c char or char sequence to be printed
 */
void raw_putc(int c);

void puts(const char *str);

void printf(const char *str);

void putc(int c);

int getc(void);

void gets(char *buffer, uint32_t size);

void vga_xor_cursor(void);

void vga_clr_cursor(void);

#endif //STDIO_H
