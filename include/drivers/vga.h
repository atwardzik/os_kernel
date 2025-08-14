//
// Created by Artur Twardzik on 24/07/2025.
//

#ifndef KERNEL_VGA_H
#define KERNEL_VGA_H

#include <stdint.h>

void hsync_gen_init(uint32_t pin);

void vsync_gen_init(uint32_t pin);

void rgb_gen_init(uint32_t pin_red0);

void setup_vga_dma(void);

#endif //KERNEL_VGA_H
