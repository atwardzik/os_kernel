//
// Created by Artur Twardzik on 21/08/2025.
//

#include "drivers/vga.h"
#include "drivers/gpio.h"
#include "drivers/time.h"
#include "font.h"
#include "ctype.h"
#include "escape_codes.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>


extern const uint8_t __vidram_start[];
const uint8_t *const vidram_start_ptr = __vidram_start;

extern void hsync_gen_init(uint32_t pin);

extern void vsync_gen_init(uint32_t pin);

extern void rgb_gen_init(uint32_t pin_red0);

extern void setup_vga_dma(void);

extern void vga_start(void);

constexpr uint8_t BACKSPACE = 0x08;
constexpr uint8_t ENDL = 0x0A;
constexpr uint8_t EMPTY_SPACE = 0x20;
constexpr uint8_t CURSOR_FULL = 0x81;
constexpr uint8_t ESC = 0x1b;
constexpr uint8_t ARROW_LEFT = 0x44;
constexpr uint8_t ARROW_RIGHT = 0x43;

enum PhysicalColorCode {
        PHYSICAL_BLACK         = 0b000000, // #000000
        PHYSICAL_RED           = 0b000010, // #aa0000
        PHYSICAL_GREEN         = 0b001001, // #55aa00
        PHYSICAL_YELLOW        = 0b001010, // #aaaa00
        PHYSICAL_BLUE          = 0b100000, // #0000aa
        PHYSICAL_MAGENTA       = 0b100010, // #aa00aa
        PHYSICAL_CYAN          = 0b101001, // #55aaaa
        PHYSICAL_WHITE         = 0b111111, // #ffffff
        PHYSICAL_DARK_GRAY     = 0b010101, // #555555
        PHYSICAL_RED_LIGHT     = 0b000011, // #ff0000
        PHYSICAL_GREEN_LIGHT   = 0b001100, // #00ff00
        PHYSICAL_YELLOW_LIGHT  = 0b001111, // #ffff00
        PHYSICAL_BLUE_LIGHT    = 0b110000, // #0000ff
        PHYSICAL_MAGENTA_LIGHT = 0b110011, // #ff00ff
        PHYSICAL_CYAN_LIGHT    = 0b111101, // #55ffff
        PHYSICAL_LIGHT_GRAY    = 0b101010, // #aaaaaa
};

void vga_init(const uint32_t hsync_pin, const uint32_t vsync_pin, const uint32_t pin_red0) {
        vga_clr_screen();

        hsync_gen_init(hsync_pin);
        vsync_gen_init(vsync_pin);
        rgb_gen_init(pin_red0);

        setup_vga_dma();

        vga_start();
}

static void set_colors_from_escape(PhysicalColor *foreground_color, PhysicalColor *background_color,
                                   const ByteColorCode escape_color_code
) {
        const uint8_t foreground_color_encoded = escape_color_code & FOREGROUND_COLOR_BITS;
        const bool foreground_color_light = foreground_color_encoded & FOREGROUND_LIGHT_COLOR_BIT;
        const uint8_t background_color_encoded = escape_color_code & BACKGROUND_COLOR_BITS;
        const bool background_color_light = background_color_encoded & BACKGROUND_LIGHT_COLOR_BIT;

        if (foreground_color_light) {
                switch (foreground_color_encoded) {
                        case BLACK:
                                *foreground_color = PHYSICAL_DARK_GRAY;
                                break;
                        case RED:
                                *foreground_color = PHYSICAL_RED_LIGHT;
                                break;
                        case GREEN:
                                *foreground_color = PHYSICAL_GREEN_LIGHT;
                                break;
                        case YELLOW:
                                *foreground_color = PHYSICAL_YELLOW_LIGHT;
                                break;
                        case BLUE:
                                *foreground_color = PHYSICAL_BLUE_LIGHT;
                                break;
                        case MAGENTA:
                                *foreground_color = PHYSICAL_MAGENTA_LIGHT;
                                break;
                        case CYAN:
                                *foreground_color = PHYSICAL_CYAN_LIGHT;
                                break;
                        case WHITE:
                                *foreground_color = PHYSICAL_LIGHT_GRAY;
                                break;
                        default:
                                *foreground_color = PHYSICAL_WHITE;
                                break;
                }
        }
        else {
                switch (foreground_color_encoded) {
                        case BLACK:
                                *foreground_color = PHYSICAL_BLACK;
                                break;
                        case RED:
                                *foreground_color = PHYSICAL_RED;
                                break;
                        case GREEN:
                                *foreground_color = PHYSICAL_GREEN;
                                break;
                        case YELLOW:
                                *foreground_color = PHYSICAL_YELLOW;
                                break;
                        case BLUE:
                                *foreground_color = PHYSICAL_BLUE;
                                break;
                        case MAGENTA:
                                *foreground_color = PHYSICAL_MAGENTA;
                                break;
                        case CYAN:
                                *foreground_color = PHYSICAL_CYAN;
                                break;
                        case WHITE:
                                *foreground_color = PHYSICAL_WHITE;
                                break;
                        default:
                                *foreground_color = PHYSICAL_WHITE;
                                break;
                }
        }

        if (background_color_light) {
                switch (background_color_encoded) {
                        case BLACK:
                                *background_color = PHYSICAL_DARK_GRAY;
                                break;
                        case RED:
                                *background_color = PHYSICAL_RED_LIGHT;
                                break;
                        case GREEN:
                                *background_color = PHYSICAL_GREEN_LIGHT;
                                break;
                        case YELLOW:
                                *background_color = PHYSICAL_YELLOW_LIGHT;
                                break;
                        case BLUE:
                                *background_color = PHYSICAL_BLUE_LIGHT;
                                break;
                        case MAGENTA:
                                *background_color = PHYSICAL_MAGENTA_LIGHT;
                                break;
                        case CYAN:
                                *background_color = PHYSICAL_CYAN_LIGHT;
                                break;
                        case WHITE:
                                *background_color = PHYSICAL_LIGHT_GRAY;
                                break;
                        default:
                                *background_color = PHYSICAL_BLACK;
                                break;
                }
        }
        else {
                switch (background_color_encoded) {
                        case BLACK:
                                *background_color = PHYSICAL_BLACK;
                                break;
                        case RED:
                                *background_color = PHYSICAL_RED;
                                break;
                        case GREEN:
                                *background_color = PHYSICAL_GREEN;
                                break;
                        case YELLOW:
                                *background_color = PHYSICAL_YELLOW;
                                break;
                        case BLUE:
                                *background_color = PHYSICAL_BLUE;
                                break;
                        case MAGENTA:
                                *background_color = PHYSICAL_MAGENTA;
                                break;
                        case CYAN:
                                *background_color = PHYSICAL_CYAN;
                                break;
                        case WHITE:
                                *background_color = PHYSICAL_WHITE;
                                break;
                        default:
                                *background_color = PHYSICAL_BLACK;
                                break;
                }
        }
}

void vga_put_physical_color_letter(const char letter, unsigned int row_letter_position,
                                   unsigned int column_letter_position,
                                   const PhysicalColor foreground_color,
                                   const PhysicalColor background_color
) {
        const uint8_t *letter_lookup = font8x8[letter];

        const unsigned int row_padding = row_letter_position * SCREEN_WIDTH;
        row_letter_position = row_letter_position * SCREEN_WIDTH * FONT_HEIGHT;
        column_letter_position = column_letter_position * FONT_WIDTH;

        const unsigned int position = row_letter_position + row_padding + column_letter_position;

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

void vga_put_byte_encoded_color_letter(const char letter, unsigned int row_letter_position,
                                       unsigned int column_letter_position,
                                       const ByteColorCode color_code
) {
        PhysicalColor foreground_color;
        PhysicalColor background_color;
        set_colors_from_escape(&foreground_color, &background_color, color_code);

        vga_put_physical_color_letter(letter, row_letter_position, column_letter_position,
                                      foreground_color,
                                      background_color);
}


static size_t screen_row_position = 0;
static size_t screen_column_position = 0;
static size_t cursor_row_position = 0;
static size_t cursor_column_position = 0;
static PhysicalColor current_foreground_color = PHYSICAL_WHITE;
static PhysicalColor current_background_color = PHYSICAL_BLACK;

void vga_putc(const int c) {
        static uint8_t escape_sequence[10] = {};
        static size_t escape_sequence_position = 0;

        if (c > 0xff) {
                const uint8_t direction = c & 0xff;

                // TODO: add proper cursor management
                if (direction == ARROW_RIGHT) {
                        screen_column_position += 1;
                }
                else if (direction == ARROW_LEFT) {
                        screen_column_position -= 1;
                }

                return;
        }

        if (escape_sequence_position || c == ESC) {
                if (c == 'm') {
                        if (escape_sequence[2] == '0' && escape_sequence_position == 3) {
                                current_foreground_color = PHYSICAL_WHITE;
                                current_background_color = PHYSICAL_BLACK;
                                escape_sequence_position = 0;
                                return;
                        }
                        escape_sequence_position = 0;
                        const ByteColorCode color_code = decode_escape_colors(escape_sequence);
                        set_colors_from_escape(&current_foreground_color, &current_background_color, color_code);
                }
                else {
                        escape_sequence[escape_sequence_position] = c;
                        escape_sequence_position += 1;
                }

                return;
        }

        if (screen_row_position == BUFFER_HEIGHT) {
                screen_row_position = 0;
                screen_column_position = 0;
        }

        if (c == BACKSPACE && screen_column_position > 0) {
                screen_column_position -= 1;
                vga_put_physical_color_letter(EMPTY_SPACE, screen_row_position, screen_column_position,
                                              current_foreground_color,
                                              current_background_color);
        }
        else if (c == BACKSPACE && screen_column_position == 0 && screen_row_position > 0) {
                screen_row_position -= 1;
                screen_column_position = BUFFER_WIDTH - 1;
                vga_put_physical_color_letter(EMPTY_SPACE, screen_row_position, screen_column_position,
                                              current_foreground_color,
                                              current_background_color);
        }
        else if (c == ENDL) {
                screen_column_position = 0;
                screen_row_position += 1;
        }
        else if (isprint(c)) {
                vga_put_physical_color_letter(c, screen_row_position, screen_column_position, current_foreground_color,
                                              current_background_color);
                screen_column_position += 1;
                if (screen_column_position > BUFFER_WIDTH - 1) {
                        screen_row_position += 1;
                        screen_column_position = 0;
                }
        }
}

void vga_update_cursor(unsigned int row, unsigned int column) {
        vga_clr_cursor();

        cursor_row_position = row;
        cursor_column_position = column;
}

void vga_xor_cursor() {
        static bool cursor_on = false;

        if (cursor_on) {
                vga_put_physical_color_letter(CURSOR_FULL, cursor_row_position, cursor_column_position,
                                              current_foreground_color,
                                              current_background_color);
                cursor_on = false;
        }
        else {
                vga_put_physical_color_letter(EMPTY_SPACE, cursor_row_position, cursor_column_position,
                                              current_foreground_color,
                                              current_background_color);
                cursor_on = true;
        }
}

void vga_clr_cursor() {
        vga_put_physical_color_letter(EMPTY_SPACE, cursor_row_position, cursor_column_position,
                                      current_foreground_color,
                                      current_background_color);
}

void vga_clr_position() {
        screen_row_position = 0;
        screen_column_position = 0;
}

void vga_clr_all() {
        vga_clr_cursor();
        vga_set_cursor_off();
        vga_clr_position();
        vga_clr_screen();
}
