//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef UART_H
#define UART_H

void uart_init(void);

void uart_putc(int c);

void uart_puts(const char *str);

const int uart_getc(void);

void uart_clr_screen(void);

#endif //UART_H
