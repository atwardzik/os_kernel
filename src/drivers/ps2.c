//
// Created by Artur Twardzik on 18/04/2026.
//

#include "ps2.h"

#include "gpio.h"
#include "pio.h"
#include "time.h"

static void ps2_cmd_request(const struct PS2Device *device) {
        clr_pin(device->clk_pin);
        clr_pin(device->dat_pin);

        init_pin_output(device->clk_pin); //clk low
        delay_us(120);
        init_pin_output(device->dat_pin); //dat low

        init_pin_input_with_pull(device->clk_pin, true); //release CLK
}

static inline uint8_t get_parity(uint8_t byte) {
        byte ^= byte >> 4;
        byte ^= byte >> 2;
        byte ^= byte >> 1;
        return (~byte) & 1;
}

static inline int send_data(const struct PS2Device *device, const uint8_t data) {
        //TODO: timeout
        for (int j = 0; j < 9; ++j) {
                while (get_pin_mask() & (1 << device->clk_pin)) {} //wait for clk low
                if ((data >> j) & 0x1) {
                        set_pin(device->dat_pin);
                }
                else {
                        clr_pin(device->dat_pin);
                }
                while (!(get_pin_mask() & (1 << device->clk_pin))) {} //wait for clk high
        }

        init_pin_input_with_pull(device->dat_pin, true);   //release DAT
        while (get_pin_mask() & (1 << device->dat_pin)) {} //wait for dat low - ACK signal
        while (get_pin_mask() & (1 << device->clk_pin)) {} //wait for clk low

        return 0;
}

static inline int wait_for_ack(const struct PS2Device *device) {
        //TODO: timeout

        uint32_t value = 0;
        while (get_pin_mask() & (1 << device->clk_pin)) {} //wait for clk low
        // here should be START bit
        while (!(get_pin_mask() & (1 << device->clk_pin))) {} //wait for clk high

        for (int i = 0; i < 9; ++i) {
                while (get_pin_mask() & (1 << device->clk_pin)) {} //wait for clk low
                value = (value << 1) | (get_pin_mask() & (1 << device->dat_pin));
                while (!(get_pin_mask() & (1 << device->clk_pin))) {} //wait for clk high
        }
}

int ps2_tx(const struct PS2Device *device, const uint8_t *data, int length) {
        set_sm_enabled(device->pio_block, device->state_machine, false);

        for (int i = 0; i < length; ++i) {
                ps2_cmd_request(device);

                const uint8_t parity = get_parity(data[i]);
                const uint16_t cmd = data[i] | (parity << 8);
                send_data(device, cmd);

                wait_for_ack(device);

                delay_us(200);
        }

        delay_us(100);
        clear_internal_and_jump(device->pio_block, device->state_machine, 0);
        set_sm_enabled(device->pio_block, device->state_machine, true);
        return 0;
}
