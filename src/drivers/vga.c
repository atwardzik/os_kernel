//
// Created by Artur Twardzik on 21/08/2025.
//

#include "drivers/vga.h"
#include "drivers/gpio.h"
#include "drivers/time.h"
#include "font.h"
#include "ctype.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>


extern const uint8_t __vidram_start[];
const uint8_t *const vidram_start_ptr = __vidram_start;

extern void hsync_gen_init(uint32_t pin);

extern void vsync_gen_init(uint32_t pin);

extern void rgb_gen_init(uint32_t pin_red0);

extern void setup_vga_dma(void);

extern void vga_clear(void);

extern void vga_start(void);

constexpr uint8_t BACKSPACE = 0x08;
constexpr uint8_t ENDL = 0x0A;
constexpr uint8_t EMPTY_SPACE = 0x20;
constexpr uint8_t CURSOR_FULL = 0x81;
constexpr uint8_t ESC = 0x1b;
constexpr uint8_t ARROW_LEFT = 0x44;
constexpr uint8_t ARROW_RIGHT = 0x43;

struct EscapeColorCode {
        uint8_t background_color;
        bool background_light;
        uint8_t foreground_color;
        bool foreground_light;
};

enum EscapeColor {
        ESCAPE_BLACK   = 0,
        ESCAPE_RED     = 1,
        ESCAPE_GREEN   = 2,
        ESCAPE_YELLOW  = 3,
        ESCAPE_BLUE    = 4,
        ESCAPE_MAGENTA = 5,
        ESCAPE_CYAN    = 6,
        ESCAPE_WHITE   = 7,
};

enum PhysicalColor {
        PHYSICAL_BLACK         = 0b000000,
        PHYSICAL_RED           = 0b000011,
        PHYSICAL_GREEN         = 0b001100,
        PHYSICAL_YELLOW        = 0b001111,
        PHYSICAL_BLUE          = 0b110000,
        PHYSICAL_MAGENTA       = 0b110011,
        PHYSICAL_CYAN          = 0b111100,
        PHYSICAL_WHITE         = 0b111111,
        PHYSICAL_DARK_GRAY     = 0b010101,
        PHYSICAL_RED_LIGHT     = 0b000001,
        PHYSICAL_GREEN_LIGHT   = 0b000100,
        PHYSICAL_YELLOW_LIGHT  = 0b000101,
        PHYSICAL_BLUE_LIGHT    = 0b010000,
        PHYSICAL_MAGENTA_LIGHT = 0b010001,
        PHYSICAL_CYAN_LIGHT    = 0b010100,
        PHYSICAL_LIGHT_GRAY    = 0b101010,
};

void vga_init(const uint32_t hsync_pin, const uint32_t vsync_pin, const uint32_t pin_red0) {
        vga_clear();

        hsync_gen_init(hsync_pin);
        vsync_gen_init(vsync_pin);
        rgb_gen_init(pin_red0);

        setup_vga_dma();

        vga_start();
}

void vga_put_letter(const char letter, unsigned int row_letter_position, unsigned int column_letter_position,
                    const Color background_color,
                    const Color foreground_color
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


static inline bool is_foreground_light_palette(const uint8_t *color_escape_sequence) {
        if (color_escape_sequence[2] - 0x30 == 1) {
                return true;
        }

        return false;
}

static inline bool is_background_light_palette(const uint8_t *color_escape_sequence) {
        if (color_escape_sequence[2] - 0x30 == 5) {
                return true;
        }

        return false;
}


static struct EscapeColorCode decode_escape_colors(const uint8_t *color_escape_sequence) {
        struct EscapeColorCode color_code;

        color_code.foreground_light = is_foreground_light_palette(color_escape_sequence);
        color_code.background_light = is_background_light_palette(color_escape_sequence);

        if (color_code.foreground_light || color_code.background_light) {
                color_code.foreground_color = color_escape_sequence[5] - 0x30;
                color_code.background_color = color_escape_sequence[8] - 0x30;
        }
        else {
                color_code.foreground_color = color_escape_sequence[3] - 0x30;
                color_code.background_color = color_escape_sequence[6] - 0x30;
        }

        return color_code;
}

static void set_colors_from_escape(Color *foreground_color, Color *background_color,
                                   const struct EscapeColorCode *escape_color_code
) {
        if (escape_color_code->foreground_light) {
                switch (escape_color_code->foreground_color) {
                        case ESCAPE_BLACK:
                                *foreground_color = PHYSICAL_DARK_GRAY;
                                break;
                        case ESCAPE_RED:
                                *foreground_color = PHYSICAL_RED_LIGHT;
                                break;
                        case ESCAPE_GREEN:
                                *foreground_color = PHYSICAL_GREEN_LIGHT;
                                break;
                        case ESCAPE_YELLOW:
                                *foreground_color = PHYSICAL_YELLOW_LIGHT;
                                break;
                        case ESCAPE_BLUE:
                                *foreground_color = PHYSICAL_BLUE_LIGHT;
                                break;
                        case ESCAPE_MAGENTA:
                                *foreground_color = PHYSICAL_MAGENTA_LIGHT;
                                break;
                        case ESCAPE_CYAN:
                                *foreground_color = PHYSICAL_CYAN_LIGHT;
                                break;
                        case ESCAPE_WHITE:
                                *foreground_color = PHYSICAL_LIGHT_GRAY;
                                break;
                        default:
                                *foreground_color = PHYSICAL_WHITE;
                                break;
                }
        }
        else {
                switch (escape_color_code->foreground_color) {
                        case ESCAPE_BLACK:
                                *foreground_color = PHYSICAL_BLACK;
                                break;
                        case ESCAPE_RED:
                                *foreground_color = PHYSICAL_RED;
                                break;
                        case ESCAPE_GREEN:
                                *foreground_color = PHYSICAL_GREEN;
                                break;
                        case ESCAPE_YELLOW:
                                *foreground_color = PHYSICAL_YELLOW;
                                break;
                        case ESCAPE_BLUE:
                                *foreground_color = PHYSICAL_BLUE;
                                break;
                        case ESCAPE_MAGENTA:
                                *foreground_color = PHYSICAL_MAGENTA;
                                break;
                        case ESCAPE_CYAN:
                                *foreground_color = PHYSICAL_CYAN;
                                break;
                        case ESCAPE_WHITE:
                                *foreground_color = PHYSICAL_WHITE;
                                break;
                        default:
                                *foreground_color = PHYSICAL_WHITE;
                                break;
                }
        }

        if (escape_color_code->background_light) {
                switch (escape_color_code->background_color) {
                        case ESCAPE_BLACK:
                                *background_color = PHYSICAL_DARK_GRAY;
                                break;
                        case ESCAPE_RED:
                                *background_color = PHYSICAL_RED_LIGHT;
                                break;
                        case ESCAPE_GREEN:
                                *background_color = PHYSICAL_GREEN_LIGHT;
                                break;
                        case ESCAPE_YELLOW:
                                *background_color = PHYSICAL_YELLOW_LIGHT;
                                break;
                        case ESCAPE_BLUE:
                                *background_color = PHYSICAL_BLUE_LIGHT;
                                break;
                        case ESCAPE_MAGENTA:
                                *background_color = PHYSICAL_MAGENTA_LIGHT;
                                break;
                        case ESCAPE_CYAN:
                                *background_color = PHYSICAL_CYAN_LIGHT;
                                break;
                        case ESCAPE_WHITE:
                                *background_color = PHYSICAL_LIGHT_GRAY;
                                break;
                        default:
                                *background_color = PHYSICAL_BLACK;
                                break;
                }
        }
        else {
                switch (escape_color_code->background_color) {
                        case ESCAPE_BLACK:
                                *background_color = PHYSICAL_BLACK;
                                break;
                        case ESCAPE_RED:
                                *background_color = PHYSICAL_RED;
                                break;
                        case ESCAPE_GREEN:
                                *background_color = PHYSICAL_GREEN;
                                break;
                        case ESCAPE_YELLOW:
                                *background_color = PHYSICAL_YELLOW;
                                break;
                        case ESCAPE_BLUE:
                                *background_color = PHYSICAL_BLUE;
                                break;
                        case ESCAPE_MAGENTA:
                                *background_color = PHYSICAL_MAGENTA;
                                break;
                        case ESCAPE_CYAN:
                                *background_color = PHYSICAL_CYAN;
                                break;
                        case ESCAPE_WHITE:
                                *background_color = PHYSICAL_WHITE;
                                break;
                        default:
                                *background_color = PHYSICAL_BLACK;
                                break;
                }
        }
}

static int screen_row_position = 0;
static int screen_column_position = 0;
static Color foreground_color = PHYSICAL_WHITE;
static Color background_color = PHYSICAL_BLACK;

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

        // TODO: implement other escape sequences than changing color
        if (escape_sequence_position || c == ESC) {
                if (c == 'm') {
                        if (escape_sequence[2] == '0' && escape_sequence_position == 3) {
                                foreground_color = PHYSICAL_WHITE;
                                background_color = PHYSICAL_BLACK;
                                escape_sequence_position = 0;
                                return;
                        }
                        escape_sequence_position = 0;
                        const struct EscapeColorCode color_code = decode_escape_colors(escape_sequence);
                        set_colors_from_escape(&foreground_color, &background_color, &color_code);
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
                vga_put_letter(EMPTY_SPACE, screen_row_position, screen_column_position, background_color,
                               foreground_color);
        }
        else if (c == BACKSPACE && screen_column_position == 0 && screen_row_position > 0) {
                screen_row_position -= 1;
                screen_column_position = BUFFER_WIDTH - 1;
                vga_put_letter(EMPTY_SPACE, screen_row_position, screen_column_position, background_color,
                               foreground_color);
        }
        else if (c == ENDL) {
                screen_column_position = 0;
                screen_row_position += 1;
        }
        else if (isprint(c)) {
                vga_put_letter(c, screen_row_position, screen_column_position, background_color,
                               foreground_color);
                screen_column_position += 1;
                if (screen_column_position > BUFFER_WIDTH - 1) {
                        screen_row_position += 1;
                        screen_column_position = 0;
                }
        }
}

void vga_xor_cursor() {
        static bool cursor_on = false;

        if (cursor_on) {
                vga_put_letter(CURSOR_FULL, screen_row_position, screen_column_position, background_color, foreground_color);
                cursor_on = false;
        }
        else {
                vga_put_letter(EMPTY_SPACE, screen_row_position, screen_column_position, background_color, foreground_color);
                cursor_on = true;
        }
}

void vga_clr_cursor() {
        vga_put_letter(EMPTY_SPACE, screen_row_position, screen_column_position, background_color, foreground_color);
}
