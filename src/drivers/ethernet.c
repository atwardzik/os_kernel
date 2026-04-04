//
// Created by Artur Twardzik on 30/03/2026.
//

#include "ethernet.h"

#include "errno.h"
#include "gpio.h"
#include "libc.h"
#include "spi.h"
#include "time.h"
#include "kernel/error.h"
#include "kernel/memory.h"

static constexpr int ETH_CS = 17;
static constexpr int ETH_SPI_BLOCK = 0;
static constexpr uint16_t S0_TX_BASE = 0x4000;

static constexpr uint16_t MR = 0x0000;
static constexpr uint16_t GAR = 0x0001;
static constexpr uint16_t SUBR = 0x0005;
static constexpr uint16_t SHAR = 0x0009;
static constexpr uint16_t SIPR = 0x000f;
static constexpr uint16_t IMR = 0x0016;
static constexpr uint16_t RTR = 0x0017;
static constexpr uint16_t RCR = 0x0019;
static constexpr uint16_t VERR_REG = 0x0080;

#define SN_MR(socket) (0x0000 + 0x0100 * ((uint16_t) socket + 4))
#define SN_CR(socket) (0x0001 + 0x0100 * ((uint16_t) socket + 4))
#define SN_IR(socket) (0x0002 + 0x0100 * ((uint16_t) socket + 4))
#define SN_TX_CR(socket) (0x0001 + 0x0100 * ((uint16_t) socket + 4))
#define SN_RXBUF_SIZE(socket) (0x001e + 0x0100 * ((uint16_t) socket + 4))
#define SN_TXBUF_SIZE(socket) (0x001f + 0x0100 * ((uint16_t) socket + 4))
#define SN_TX_FSR(socket) (0x0020 + 0x0100 * ((uint16_t) socket + 4))
#define SN_TX_RD(socket) (0x0022 + 0x0100 * ((uint16_t) socket + 4))
#define SN_TX_WR(socket) (0x0024 + 0x0100 * ((uint16_t) socket + 4))

#define SENDOK 0x16
#define TIMEOUT 0x08


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

        spi_init(0, 2, 1, 0);
}

/**
 * Read from hw register
 * @param reg_offset specified hw register's offset
 * @param data array of n bytes to be sent
 * @param len length of the bytes buffer
 */
static void eth_write_reg(uint16_t reg_offset, const uint8_t *data, const size_t len) {
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

static void eth_read_reg(uint16_t reg_offset, uint8_t *buf, const size_t len) {
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

static bool is_chip_compatible(void) {
        uint8_t reg[1];
        eth_read_reg(VERR_REG, reg, 1);

        if (reg[0] == 0x51) {
                return true;
        }

        return false;
}

static void setup_communication_details(void) {
        constexpr uint8_t cmd_set_mr[8] = {0x00};
        eth_write_reg(MR, cmd_set_mr, sizeof(cmd_set_mr));

        //IMR = 0 - mask all interrupts, polling mode
        constexpr uint8_t cmd_set_imr[8] = {0x00};
        eth_write_reg(IMR, cmd_set_imr, sizeof(cmd_set_imr));

        //RTR = 0 - 200ms retransmission timeout
        constexpr uint8_t cmd_set_rtr[8] = {0x07, 0xd0};
        eth_write_reg(RTR, cmd_set_rtr, sizeof(cmd_set_rtr));

        //RCR = 0 - 8 retries before giving up
        constexpr uint8_t cmd_set_rcr[8] = {0x08};
        eth_write_reg(RCR, cmd_set_rcr, sizeof(cmd_set_rcr));
}

static int setup_interface_information(struct NetworkInterface *interface) {
        eth_write_reg(SHAR, interface->mac, sizeof(interface->mac));

        eth_write_reg(GAR, interface->gateway, sizeof(interface->gateway));

        eth_write_reg(SUBR, interface->subnet_mask, sizeof(interface->subnet_mask));

        eth_write_reg(SIPR, interface->ip, sizeof(interface->ip));

        // check if the values are set up properly?
        return 0;
}

static int open_socket(struct NetworkInterface *interface, const int socket_number, const enum SocketMode mode) {
        if (mode == MACRAW && socket_number != 0) {
                return -ESOCKTNOSUPPORT;
        }

        const uint8_t socket_mode[] = {mode};
        eth_write_reg(SN_MR(socket_number), socket_mode, 1);

        constexpr uint8_t open[] = {OPEN};
        eth_write_reg(SN_CR(socket_number), open, 1);

        return 0;
}

static uint16_t determine_sn_tx_buffer(const int socket_number) {
        if (socket_number != 0) {
                UNIMPLEMENTED("WIZnet W5100s driver has implemented only socket 0.");
        }

        return S0_TX_BASE;
}

static int tx_raw_frame(
        struct NetworkInterface *interface, const int socket_number, const char *frame, const size_t frame_size
) {
        const struct Socket *socket = &interface->sockets[socket_number];
        //TODO: socket validation

        uint8_t offsets[2];
        eth_read_reg(SN_TX_RD(socket_number), offsets, 2);
        const uint16_t current_offset = ((offsets[0] << 8) | (offsets[1])) & socket->socket_txbuf_mask;

        uint8_t free_space[2];
        eth_read_reg(SN_TX_FSR(socket_number), free_space, 2);
        const uint16_t current_free_space = (free_space[0] << 8) | (free_space[1]); //available socket space
        if (frame_size > current_free_space) {
                return -EMSGSIZE;
        }

        // Write frame to TX buffer
        if (current_offset + frame_size > socket->socket_txbuf_size_max) {
                const size_t rest_size = socket->socket_txbuf_size_max - current_offset;
                eth_write_reg(determine_sn_tx_buffer(socket_number) + current_offset, frame, rest_size);

                eth_write_reg(determine_sn_tx_buffer(socket_number), frame + rest_size, frame_size - rest_size);
        }
        else {
                eth_write_reg(determine_sn_tx_buffer(socket_number) + current_offset, frame, frame_size);
        }

        // Set TX write pointer
        const uint16_t tx_wr = current_offset + frame_size;
        const uint8_t ptr[] = {tx_wr >> 8, tx_wr & 0xff};
        eth_write_reg(SN_TX_WR(socket_number), ptr, 2);

        // Send command
        constexpr uint8_t send[] = {SEND};
        eth_write_reg(SN_TX_CR(socket_number), send, 1);

        // Wait until SEND Command is cleared
        uint8_t cr[1];
        do {
                eth_read_reg(SN_TX_CR(socket_number), cr, 1);
        } while (cr[0] != 0x00);

        // Wait until SEND Command is completed or TIMEOUT Interrupt is occurred
        uint8_t ir[1];
        do {
                eth_read_reg(SN_IR(socket_number), ir, 1);

                ir[0] &= SENDOK | TIMEOUT;
        } while (!ir[0]);

        if (ir[0] & SENDOK) {
                constexpr uint8_t clear[] = {SENDOK};
                eth_write_reg(SN_IR(socket_number), clear, 1);
        }
        else {
                //timeout
        }

        return 0;
}

struct NetworkInterface *setup_ethernet_chip(void) {
        setup_eth_spi();
        if (!is_chip_compatible()) {
                return ERR_PTR(-ENODEV);
        }

        //reset
        constexpr uint8_t cmd_rst[] = {0x80};
        eth_write_reg(MR, cmd_rst, sizeof(cmd_rst));
        delay_ms(64); //stable time from the datasheet

        setup_communication_details();

        struct NetworkInterface *interface = (struct NetworkInterface *) kmalloc(sizeof(*interface));
        struct NetworkInterfaceOperations *i_op = (struct NetworkInterfaceOperations *) kmalloc(sizeof(*i_op));
        i_op->setup_interface_information = setup_interface_information;
        i_op->open_socket = open_socket;
        i_op->tx_raw_frame = tx_raw_frame;
        interface->i_op = i_op;

        struct Socket *sockets = (struct Socket *) kmalloc(4 * sizeof(struct Socket));
        //sockets memory - 4KB per socket (0x02 = 2KB, 0x04 = 4KB, 0x08 = 8KB)
        for (int i = 0; i < 4; i++) {
                constexpr uint8_t cmd[] = {0x04};
                eth_write_reg(SN_TXBUF_SIZE(i), cmd, sizeof(cmd)); // 4KB TX
                eth_write_reg(SN_RXBUF_SIZE(i), cmd, sizeof(cmd)); // 4KB RX

                const struct Socket socket = {4096, 0x0fff};
                sockets[i] = socket;
        }
        interface->sockets = sockets;

        return interface;
}
