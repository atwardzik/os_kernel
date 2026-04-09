//
// Created by Artur Twardzik on 09/04/2026.
//

#ifndef OS_SD_CARD_H
#define OS_SD_CARD_H
#include <stdint.h>

int init_sd_card(void);

int sd_card_read512_block(uint32_t block_number, char *buffer);

int sd_card_write512_block(uint32_t block_number, char *buffer);

#endif //OS_SD_CARD_H
