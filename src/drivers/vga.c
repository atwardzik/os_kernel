//
// Created by Artur Twardzik on 21/08/2025.
//

#include "drivers/vga.h"
#include "drivers/gpio.h"
#include "drivers/time.h"

#include <stdint.h>
#include <stddef.h>


extern const uint8_t __vidram_start[];
const uint8_t *const vidram_start_ptr = __vidram_start;

extern void hsync_gen_init(uint32_t pin);

extern void vsync_gen_init(uint32_t pin);

extern void rgb_gen_init(uint32_t pin_red0);

extern void setup_vga_dma(void);

extern void vga_clear(void);

extern void vga_start(void);


void vga_init(const uint32_t hsync_pin, const uint32_t vsync_pin, const uint32_t pin_red0) {
        vga_clear();

        hsync_gen_init(hsync_pin);
        vsync_gen_init(vsync_pin);
        rgb_gen_init(pin_red0);

        setup_vga_dma();

        vga_start();
}

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
                                *(uint8_t *) (vidram_start_ptr + i * SCREEN_WIDTH + j + position) =
                                                foreground_color;
                        }
                        else {
                                *(uint8_t *) (vidram_start_ptr + i * SCREEN_WIDTH + j + position) =
                                                background_color;
                        }
                }
        }
}
