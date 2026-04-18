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
        uint8_t counter = 0;

        while (byte) {
                counter += byte & 1;
                byte >>= 1;
        }

        return !(counter % 2);
}

static inline int send_data(const struct PS2Device *device, const uint16_t data) {
        //TODO: timeout
        int i = 0;
        while (get_pin_mask() & (1 << device->clk_pin)) {} //wait for clk low

        do {
                if ((data >> i) & 0x1) {
                        init_pin_input_with_pull(device->dat_pin, true);
                }
                else {
                        clr_pin(device->dat_pin);
                        init_pin_output(device->dat_pin);
                }

                while (!(get_pin_mask() & (1 << device->clk_pin))) {} //wait for clk high
                while (get_pin_mask() & (1 << device->clk_pin)) {}    //wait for clk low

                i += 1;
        } while (i < 9);

        init_pin_input_with_pull(device->dat_pin, true);   //release DAT
        while (get_pin_mask() & (1 << device->dat_pin)) {} //wait for dat low - ACK signal
        while (get_pin_mask() & (1 << device->clk_pin)) {} //wait for clk low

        return 0;
}

static inline uint32_t wait_for_ack(const struct PS2Device *device) {
        //TODO: timeout

        uint32_t value = 0;
        while (get_pin_mask() & (1 << device->clk_pin)) {} //wait for clk low
        // here should be START bit
        while (!(get_pin_mask() & (1 << device->clk_pin))) {} //wait for clk high

        for (int i = 0; i < 8; ++i) {
                while (get_pin_mask() & (1 << device->clk_pin)) {} //wait for clk low

                const uint32_t mask = (get_pin_mask() & (1 << device->dat_pin)) ? 1 : 0;
                value |= (mask << i);

                while (!(get_pin_mask() & (1 << device->clk_pin))) {} //wait for clk high
        }

        while (get_pin_mask() & (1 << device->clk_pin)) {} //wait for clk low
        // here should be PARITY bit
        while (!(get_pin_mask() & (1 << device->clk_pin))) {} //wait for clk high
        while (get_pin_mask() & (1 << device->clk_pin)) {}    //wait for clk low
        // here should be STOP bit
        while (!(get_pin_mask() & (1 << device->clk_pin))) {} //wait for clk high

        return value;
}

static int wait_for_clk_dat_release(const struct PS2Device *device) {
        while (!(get_pin_mask() & (1 << device->dat_pin))) {} //wait for dat high
        while (!(get_pin_mask() & (1 << device->clk_pin))) {} //wait for clk high
}

int ps2_tx(const struct PS2Device *device, const uint8_t *data, int length) {
        const bool running = is_pio_sm_running(device->pio_block, device->state_machine);
        if (running) {
                set_sm_enabled(device->pio_block, device->state_machine, false);
        }

        for (int i = 0; i < length; ++i) {
                const uint8_t parity = get_parity(data[i]);
                const uint16_t cmd = data[i] | (parity << 8);

                ps2_cmd_request(device);

                send_data(device, cmd);

                wait_for_ack(device);
                wait_for_clk_dat_release(device);

                delay_ms(1);
        }

        delay_us(100);

        if (running) {
                clear_internal_and_jump(device->pio_block, device->state_machine, 0);
                set_sm_enabled(device->pio_block, device->state_machine, true);
        }
        return 0;
}
