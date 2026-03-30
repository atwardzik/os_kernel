//
// Created by Artur Twardzik on 30/03/2026.
//

#include "ethernet.h"

#include <sys/errno.h>

#include "gpio.h"
#include "spi.h"
#include "time.h"


enum EthOperation {
        READ,
        WRITE,
};

static constexpr int ETH_CS = 17;
static constexpr int ETH_SPI_BLOCK = 0;

static constexpr uint16_t VERR_REG = 0x0080;

/**
 * Read from hw register
 * @param reg_offset specified hw register's offset
 * @param data array of n bytes to be sent
 * @param len length of the bytes buffer
 */
static void eth_write_reg(uint16_t reg_offset, const char *data, const size_t len) {
        for (size_t i = 0; i < len; i++) {
                clr_pin(ETH_CS);

                spi_tx(ETH_SPI_BLOCK, 0xf0);
                spi_tx(ETH_SPI_BLOCK, reg_offset >> 8);
                spi_tx(ETH_SPI_BLOCK, reg_offset & 0x00ff);

                spi_tx(ETH_SPI_BLOCK, data[i]);

                reg_offset += 1;
                set_pin(ETH_CS);
        }
}

static void eth_read_reg(uint16_t reg_offset, char *buf, const size_t len) {
        for (size_t i = 0; i < len; i++) {
                clr_pin(ETH_CS);
                spi_tx(ETH_SPI_BLOCK, 0x0f);

                spi_tx(ETH_SPI_BLOCK, reg_offset >> 8);
                spi_tx(ETH_SPI_BLOCK, reg_offset & 0x00ff);

                buf[i] = spi_rx(ETH_SPI_BLOCK);

                reg_offset += 1;
                set_pin(ETH_CS);
        }
}

static void setup_eth_spi(void) {
        //spi0 rx
        GPIO_function_select(16, 1);
        output_enable_pin(16);

        //spi0 sck
        GPIO_function_select(18, 1);
        output_enable_pin(18);

        //spi0 tx
        GPIO_function_select(19, 1);
        output_enable_pin(19);

        //manual CS
        init_pin_output(ETH_CS);

        spi_init(0, 2, 15, 0);
}

static bool is_chip_compatible(void) {
        char reg[1];
        eth_read_reg(VERR_REG, reg, 1);

        if (reg[0] == 0x51) {
                return true;
        }

        return false;
}

int setup_ethernet(const char *ip_address, const char *mac_address, const char *gateway, uint32_t subnet_mask) {
        setup_eth_spi();
        if (!is_chip_compatible()) {
                return -ENODEV;
        }

        //reset
        constexpr char cmd_rst[] = {0x80};
        eth_write_reg(0x0000, cmd_rst, sizeof(cmd_rst));
        delay_ms(64); //stable time from the datasheet

        //MR = 0
        constexpr char cmd_set_mr[8] = {0x00};
        eth_write_reg(0x0000, cmd_set_mr, sizeof(cmd_set_mr));

        //IMR = 0 - mask all interrupts, polling mode
        constexpr char cmd_set_imr[8] = {0x00};
        eth_write_reg(0x0016, cmd_set_imr, sizeof(cmd_set_imr));

        //RTR = 0 - 200ms retransmission timeout
        constexpr char cmd_set_rtr[8] = {0x07, 0xd0};
        eth_write_reg(0x0017, cmd_set_rtr, sizeof(cmd_set_rtr));

        //RCR = 0 - 8 retries before giving up
        constexpr char cmd_set_rcr[8] = {0x08};
        eth_write_reg(0x0019, cmd_set_rcr, sizeof(cmd_set_rcr));

        //Set MAC
        char mac[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0x01};
        eth_write_reg(0x0009, mac, sizeof(mac));

        //Gateway
        constexpr char gw[] = {192, 168, 1, 1};
        eth_write_reg(0x0001, gw, sizeof(gw));

        //Subnet
        constexpr char sub[] = {255, 255, 255, 0};
        eth_write_reg(0x0005, sub, sizeof(sub));

        //IP
        constexpr char ip[] = {192, 168, 1, 100};
        eth_write_reg(0x000f, ip, sizeof(ip));

        //sockets
        // 4KB per socket (0x02 = 2KB, 0x04 = 4KB, 0x08 = 8KB)
        for (int i = 0; i < 4; i++) {
                constexpr char cmd[] = {0x04};
                eth_write_reg(0x001f + 0x0100 * (i + 4), cmd, sizeof(cmd)); // 4KB TX
        }
        for (int i = 0; i < 4; i++) {
                constexpr char cmd[] = {0x04};
                eth_write_reg(0x001e + 0x0100 * (i + 4), cmd, sizeof(cmd)); // 4KB RX
        }

        // Open socket 0 in MACRAW mode
        constexpr char mode[] = {0x04}; // MACRAW
        eth_write_reg(0x0400, mode, 1); // Sn_MR

        constexpr char open[] = {0x01}; // OPEN command
        eth_write_reg(0x0401, open, 1); // Sn_CR

        //send raw bytes
        char offsets[3] = {}; //tx read pointer
        eth_read_reg(0x0422, offsets, 2);
        //experimental sending a frame
        char frame[] = {
                0xde, 0xad, 0xbe, 0xef, 0x00, 0x02, // destination MAC
                0xde, 0xda, 0xbe, 0xba, 0xfe, 0xfa, // source MAC
                0x88, 0xb5,                         // EtherType - experimental
                'h', 'e', 'l', 'l', 'o'             // data
        };

        // Write frame to TX buffer
        eth_write_reg(0x4000, frame, sizeof(frame)); // S0 TX buffer base

        // Set TX write pointer
        uint16_t tx_wr = sizeof(frame);
        char ptr[] = {tx_wr >> 8, tx_wr & 0xff};
        eth_write_reg(0x0424, ptr, 2); // Sn_TX_WR

        // Send command
        char send[] = {0x20};           // SEND command
        eth_write_reg(0x0401, send, 1); // Sn_CR

        char cr[1] = {0x20};
        while (cr[0] != 0x00) {
                eth_write_reg(0x0401, cr, 1);
        }

        return 0;
}
