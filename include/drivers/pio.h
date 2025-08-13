//
// Created by Artur Twardzik on 28/07/2025.
//

#ifndef PIO_H
#define PIO_H

#include <stdint.h>

int load_pio_prog(uint32_t program_src, uint32_t program_size, uint32_t pio_block);

int sm_put(uint32_t pio_block, uint32_t state_machine, uint32_t word);

#endif //PIO_H
