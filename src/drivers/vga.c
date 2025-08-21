//
// Created by Artur Twardzik on 21/08/2025.
//

#include "drivers/vga.h"

#include <stddef.h>

#define VIDRAM_ADDRESS_BEGIN ((void *) 0x2003'5000)

void vga_put_letter(const char letter, unsigned int row_position, unsigned int column_position,
                    const Color background_color,
                    const Color foreground_color
) {
        const uint8_t *letter_lookup = font8x8[letter];

        const unsigned int row_padding = row_position * SCREEN_WIDTH;
        row_position = row_position * SCREEN_WIDTH * FONT_HEIGHT;
        column_position = column_position * FONT_WIDTH;

        const unsigned int position = row_position + row_padding + column_position;

        for (size_t i = 0; i < 8; ++i) {
                const uint8_t pixel_line = letter_lookup[i];
                for (size_t j = 0; j < 8; ++j) {
                        if (pixel_line & (1 << j)) {
                                *(uint8_t *) (VIDRAM_ADDRESS_BEGIN + i * SCREEN_WIDTH + j + position) = foreground_color;
                        }
                        else {
                                *(uint8_t *) (VIDRAM_ADDRESS_BEGIN + i * SCREEN_WIDTH + j + position) = background_color;
                        }
                }
        }
}
