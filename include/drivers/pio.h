//
// Created by Artur Twardzik on 28/07/2025.
//

#ifndef PIO_H
#define PIO_H

#include <stdint.h>

int load_pio_prog(uint32_t program_src, uint32_t program_size, int pio_block);

int sm_put(int pio_block, int state_machine, uint32_t word);

void set_sm_enabled(int pio_block, int state_machine, bool enabled);

void clear_internal_and_jump(int pio_block, int state_machine, uint8_t address_begin);

#endif //PIO_H
