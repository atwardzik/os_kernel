//
// Created by Artur Twardzik on 24/07/2025.
//

#ifndef KERNEL_VGA_H
#define KERNEL_VGA_H

#include <stdint.h>

constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 480;

constexpr int BUFFER_WIDTH = 80;
constexpr int BUFFER_HEIGHT = 40;

typedef uint8_t Color;

void vga_init(uint32_t hsync_pin, uint32_t vsync_pin, uint32_t pin_red0);

void vga_put_letter(char letter, unsigned int row_letter_position, unsigned int column_letter_position,
                    Color background_color,
                    Color foreground_color
);

void vga_put_pixel(Color color, unsigned int row_pixel_position, unsigned int column_pixel_position);

/**
 * Prints single character onto the screen. \n
 *
 * As for escape sequences only changing colors are supported, as the font is 8x8. \n
 * The color code format must be 0x1b 0x5b <fg> 0x3b <bg> 0x6d, where
 *  - <fg> is in range 30-37 or 90-97 (bright colors),
 *  - <bg> is in range 40-47 or 100-107 (bright colors).
 *
 * @param c char or char sequence to be printed
 */
void vga_putc(int c);

extern void vga_set_cursor_blink(uint32_t us);

extern void vga_set_cursor_off(void);

void vga_xor_cursor(void);

void vga_clr_cursor(void);

void vga_clr_position(void);

void vga_clr_screen(void);

void vga_clr_all(void);

#endif // KERNEL_VGA_H
