//
// Created by Artur Twardzik on 18/04/2026.
//

#ifndef OS_PS2_H
#define OS_PS2_H
#include <stdint.h>

struct PS2Device {
        unsigned int clk_pin;
        unsigned int dat_pin;

        unsigned int pio_block;
        unsigned int state_machine;
};

int ps2_tx(const struct PS2Device *device, const uint8_t *data, int length);

#endif //OS_PS2_H
