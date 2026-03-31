//
// Created by Artur Twardzik on 30/03/2026.
//

#include "ethernet.h"

#include "errno.h"
#include "gpio.h"
#include "libc.h"
#include "spi.h"
#include "time.h"
#include "kernel/memory.h"


static constexpr int ETH_CS = 17;
static constexpr int ETH_SPI_BLOCK = 0;

static constexpr uint16_t VERR_REG = 0x0080;

enum SocketCommand {
        OPEN      = 0x01,
        LISTEN    = 0x02,
        CONNECT   = 0x04,
        DISCON    = 0x08,
        CLOSE     = 0x10,
        SEND      = 0x20,
        SEND_MAC  = 0x21,
        SEND_KEEP = 0x22,
        RECV      = 0x40,
};

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

static void setup_communication_details(void) {
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
}

void setup_network_information(
        const char *ip_address, const char *mac_address, const char *gateway, uint32_t subnet_mask
) {
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
}

int setup_ethernet_chip(void) {
        setup_eth_spi();
        if (!is_chip_compatible()) {
                return -ENODEV;
        }

        //reset
        constexpr char cmd_rst[] = {0x80};
        eth_write_reg(0x0000, cmd_rst, sizeof(cmd_rst));
        delay_ms(64); //stable time from the datasheet

        setup_communication_details();

        //sockets memory
        // 4KB per socket (0x02 = 2KB, 0x04 = 4KB, 0x08 = 8KB)
        for (int i = 0; i < 4; i++) {
                constexpr char cmd[] = {0x04};
                eth_write_reg(0x001f + 0x0100 * (i + 4), cmd, sizeof(cmd)); // 4KB TX
        }
        for (int i = 0; i < 4; i++) {
                constexpr char cmd[] = {0x04};
                eth_write_reg(0x001e + 0x0100 * (i + 4), cmd, sizeof(cmd)); // 4KB RX
        }


        return 0;
}

int open_socket(int socket_number, enum SocketMode mode) {
        if (mode == MACRAW && socket_number != 0) {
                return -ESOCKTNOSUPPORT;
        }

        const uint16_t sn_mr = 0x0000 + 0x0100 * (socket_number + 4);
        const char socket_mode[] = {mode};
        eth_write_reg(sn_mr, socket_mode, 1);

        const uint16_t sn_cr = 0x0001 + sn_mr;
        constexpr char open[] = {OPEN};
        eth_write_reg(sn_cr, open, 1);

        return 0;
}

static int str2mac(const char *src_mac, char *buf) {
        if (strlen(src_mac) != 17) {
                return -EINVAL;
        }

        char mac[18];
        strcpy(mac, src_mac);

        char *byte = strtok(mac, ":");
        int i = 0;
        while (i < 6) {
                buf[i] = strtoul(byte, nullptr, 16);
                byte = strtok(nullptr, ":");

                i += 1;
        }

        return 0;
}

int send_raw_frame(const char *src_mac, const char *dst_mac, uint16_t ether_type, const char *data) {
        char offsets[3] = {}; //tx read pointer
        eth_read_reg(0x0422, offsets, 2);

        char src[6];
        char dst[6];
        str2mac(src_mac, src);
        str2mac(dst_mac, dst);
        const size_t frame_size = 18 + strlen(data) + 1;
        char *frame = (char *) kmalloc(frame_size);
        memcpy(frame, src, 6);
        memcpy(frame + 6, dst, 6);
        frame[12] = ether_type >> 8;
        frame[13] = ether_type & 0xff;
        strcpy(frame + 14, data);
        frame[frame_size] = 0;

        // Write frame to TX buffer
        eth_write_reg(0x4000, frame, frame_size); // S0 TX buffer base

        // Set TX write pointer
        uint16_t tx_wr = frame_size;
        char ptr[] = {tx_wr >> 8, tx_wr & 0xff};
        eth_write_reg(0x0424, ptr, 2); // Sn_TX_WR

        // Send command
        char send[] = {0x20};           // SEND command
        eth_write_reg(0x0401, send, 1); // Sn_CR}
