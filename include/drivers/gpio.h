//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

void init_pin_output(uint32_t pin);

void xor_pin(uint32_t pin);

void set_pin(uint32_t pin);

void clr_pin(uint32_t pin);

void output_enable_pin(uint32_t pin);

void GPIO_function_select(uint32_t pin, uint32_t function);

void init_pin_input_with_pull(uint32_t pin, bool pullup);

void set_irq_pin_enabled_edge_low(uint32_t pin);

void clr_interrupt(uint32_t pin);

#endif //GPIO_H
