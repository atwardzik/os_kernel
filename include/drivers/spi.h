//
// Created by Artur Twardzik on 24/01/2026.
//

#ifndef OS_SPI_H
#define OS_SPI_H

#include <stdint.h>

void spi_init(uint32_t spi_address, uint32_t prescaler, uint32_t post_divider, uint32_t operating_mode);

uint32_t spi_determine_block(int spi_block);

void spi_tx(int spi_block, uint16_t data);

uint16_t spi_rx(int spi_block);

#endif //OS_SPI_H