//
// Created by Artur Twardzik on 18/04/2026.
//

#include "ps2_keyboard.h"

#include "config.h"
#include "gpio.h"
#include "ps2.h"
#include "time.h"
#include "kernel/error.h"
#include "kernel/memory.h"

constexpr int PS2_KBRD_RST = 0xff;
constexpr int PS2_KBRD_SET_LED = 0xed;

extern void init_ps2_keyboard_interface(uint32_t data_pin, uint32_t clock_pin);

static struct PS2Keyboard {
        struct PS2Device interface;

        uint8_t led_status;
} *ps2_keyboard;


enum KeyboardLED {
        CAPS_LOCK   = 0,
        NUM_LOCK    = 1,
        SCROLL_LOCK = 2,
};

int ps2_keyboard_toggle_led(const enum KeyboardLED led) {
        uint8_t led_toggled = 0;
        switch (led) {
                case CAPS_LOCK:
                        led_toggled = 4;
                        break;
                case NUM_LOCK:
                        led_toggled = 2;
                        break;
                case SCROLL_LOCK:
                        led_toggled = 1;
                        break;
        }

        ps2_keyboard->led_status ^= led_toggled;

        const uint8_t cmd[] = {PS2_KBRD_SET_LED, ps2_keyboard->led_status};
        return ps2_tx(&ps2_keyboard->interface, cmd, 2);
}

int init_ps2_keyboard(void) {
        if (!kconf->io_dev.ps2_keyboard.enabled) {
                return -ENODEV;
        }

        ps2_keyboard = kmalloc(sizeof(*ps2_keyboard));
        if (!ps2_keyboard) {
                return ENOMEM;
        }

        ps2_keyboard->interface.clk_pin = kconf->io_dev.ps2_keyboard.clk_pin;
        ps2_keyboard->interface.dat_pin = kconf->io_dev.ps2_keyboard.dat_pin;
        ps2_keyboard->interface.pio_block = 1; //TODO: dynamic detection? Depends on init_ps2_keyboard_interface
        ps2_keyboard->interface.state_machine = 0;
        ps2_keyboard->led_status = 0;


        const uint8_t cmd = PS2_KBRD_RST;
        ps2_tx(&ps2_keyboard->interface, &cmd, 1);

        init_ps2_keyboard_interface(kconf->io_dev.ps2_keyboard.dat_pin, kconf->io_dev.ps2_keyboard.clk_pin);

        return 0;
}
