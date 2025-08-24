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

#endif //GPIO_H
