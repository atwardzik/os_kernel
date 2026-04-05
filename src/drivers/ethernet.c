//
// Created by Artur Twardzik on 30/03/2026.
//

#include "ethernet.h"

#include "errno.h"
#include "gpio.h"
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
#define SN_SR(socket) (0x0003 + 0x0100 * ((uint16_t) socket + 4))
#define SN_PORTR(socket) (0x0004 + 0x0100 * ((uint16_t) socket + 4))
#define SN_TX_CR(socket) (0x0001 + 0x0100 * ((uint16_t) socket + 4))
#define SN_RXBUF_SIZE(socket) (0x001e + 0x0100 * ((uint16_t) socket + 4))
#define SN_TXBUF_SIZE(socket) (0x001f + 0x0100 * ((uint16_t) socket + 4))
#define SN_TX_FSR(socket) (0x0020 + 0x0100 * ((uint16_t) socket + 4))
#define SN_TX_RD(socket) (0x0022 + 0x0100 * ((uint16_t) socket + 4))
#define SN_TX_WR(socket) (0x0024 + 0x0100 * ((uint16_t) socket + 4))

#define SENDOK 0x16
#define TIMEOUT 0x08
#define RECV 0x04
#define DISCON 0x02
#define CON 0x01

enum SocketStatus {
        SOCK_CLOSED      = 0x00,
        SOCK_INIT        = 0x13, /*socket opened as TCP mode*/
        SOCK_LISTEN      = 0x14, /*socket is TCP mode and wait for peer connection*/
        SOCK_ESTABLISHED = 0x17, /*socket is TCP mode and TCP connection is done*/
        SOCK_CLOSE_WAIT  = 0x1c, /*socket is TCP mode and received disconnection request*/
        SOCK_UDP         = 0x22, /*socket opened as UDP mode*/
        SOCK_IPRAW       = 0x32, /*socket opened as IPRAW mode*/
        SOCK_MACRAW      = 0x42, /*socket opened as MACRAW mode*/
};

enum SocketCommand {
        CMD_OPEN      = 0x01,
        CMD_LISTEN    = 0x02,
        CMD_CONNECT   = 0x04,
        CMD_DISCON    = 0x08,
        CMD_CLOSE     = 0x10,
        CMD_SEND      = 0x20,
        CMD_SEND_MAC  = 0x21,
        CMD_SEND_KEEP = 0x22,
        CMD_RECV      = 0x40,
};

struct WIZnetSocket {
        struct Socket socket;
        uint16_t txbuf_start;

        unsigned int index;
};

/**
 * Sets up SPI interface for communication with WIZnet W5100S
 */
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


/**
 * Writes to wiznet register
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

/**
 * Reads from wiznet register
 * @param reg_offset specified hw resiters's offset
 * @param buf buffer where the data will be written into
 * @param len length of data to be read
 */
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

/**
 * Sets up interrupt mode, retransmission timeout and retries count
 */
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

/**
 * Copies network interface information from kernel struct to W5100S register
 * @param interface kernel struct with interface info
 * @return 0 on success
 */
static int setup_interface_information(struct NetworkInterface *interface) {
        eth_write_reg(SHAR, interface->mac, sizeof(interface->mac));

        eth_write_reg(GAR, interface->gateway, sizeof(interface->gateway));

        eth_write_reg(SUBR, interface->subnet_mask, sizeof(interface->subnet_mask));

        eth_write_reg(SIPR, interface->ip, sizeof(interface->ip));

        // check if the values are set up properly?
        return 0;
}


/**
 * Tries to open socket in specified mode
 * @param interface kernel struct with interface info
 * @param socket_number selected socket number
 * @param mode specified socket mode
 * @return socket number on success; -ESOCKTNOSUPPORT on failure to open socket in specified mode
 */
static int open_socket(struct NetworkInterface *interface, const int socket_number, const enum SocketMode mode) {
        if (mode == MACRAW && socket_number != 0) {
                return -ESOCKTNOSUPPORT;
        }

open:
        const uint8_t socket_mode[] = {mode};
        eth_write_reg(SN_MR(socket_number), socket_mode, 1);

        constexpr uint8_t open[] = {CMD_OPEN};
        eth_write_reg(SN_CR(socket_number), open, 1);

        uint8_t cr[1];
        do {
                eth_read_reg(SN_CR(socket_number), cr, 1);
        } while (cr[0] != 0x00); // wait until OPEN Command is cleared

        uint8_t sr[1];
        eth_read_reg(SN_SR(socket_number), sr, 1);
        const enum SocketStatus status = sr[0];
        if (!((status == SOCK_INIT && mode == TCP) || (status == SOCK_MACRAW && mode == MACRAW))) {
                goto open;
        }

        interface->sockets[socket_number]->mode = mode;

        return socket_number;
}

static int bind_socket(struct NetworkInterface *interface, const int socket_number, const uint16_t port) {
        struct WIZnetSocket *socket = (struct WIZnetSocket *) interface->sockets[socket_number];

        uint8_t portr[] = {port >> 8, port & 0xff};
        eth_write_reg(SN_PORTR(socket->index), portr, 2);

        socket->socket.port = port;
        return 0;
}

static int listen_socket(struct NetworkInterface *interface, const int socket_number) {
        struct WIZnetSocket *socket = (struct WIZnetSocket *) interface->sockets[socket_number];

        constexpr uint8_t listen[] = {CMD_LISTEN};
        eth_write_reg(SN_CR(socket->index), listen, 1);

        uint8_t cr[1];
        do {
                eth_read_reg(SN_CR(socket->index), cr, 1);
        } while (cr[0] != 0x00); // wait until LISTEN Command is cleared

        uint8_t sr[1];
        eth_read_reg(SN_SR(socket->index), sr, 1);
        const enum SocketStatus status = sr[0];
        if (status != SOCK_LISTEN) {
                return -ENOTCONN; //maybe better code?
        }

        return 0;
}

static int accept_socket(struct NetworkInterface *interface, const int socket_number) {
        enum SocketStatus status = SOCK_LISTEN;
        do {
                uint8_t sr[1];
                eth_read_reg(SN_SR(socket_number), sr, 1);
                status = sr[0];
        } while (status != SOCK_ESTABLISHED && status != SOCK_CLOSED);

        if (status == SOCK_ESTABLISHED) {
                return 0;
        }

        return -ENOTCONN;
}

static void socket_send(const struct WIZnetSocket *socket) {
        constexpr uint8_t send[] = {CMD_SEND};
        eth_write_reg(SN_TX_CR(socket->index), send, 1);

        // Wait until SEND Command is cleared
        uint8_t cr[1];
        do {
                eth_read_reg(SN_TX_CR(socket->index), cr, 1);
        } while (cr[0] != 0x00);
}

static int wait_send_complete(const struct WIZnetSocket *socket) {
        uint8_t ir[1];
        do {
                eth_read_reg(SN_IR(socket->index), ir, 1);

                ir[0] &= SENDOK | TIMEOUT;
        } while (!ir[0]);

        if (ir[0] & SENDOK) {
                constexpr uint8_t clear[] = {SENDOK};
                eth_write_reg(SN_IR(socket->index), clear, 1);
        }
        else {
                return -ETIMEDOUT;
        }

        return 0;
}

static void set_socket_tx_wr(struct WIZnetSocket *socket, const size_t frame_size) {
        uint8_t offsets[2];
        eth_read_reg(SN_TX_WR(socket->index), offsets, 2);
        uint16_t new_offset = ((offsets[0] << 8) | (offsets[1])) + frame_size;

        const uint8_t wr_ptr[] = {new_offset >> 8, new_offset & 0xff};
        eth_write_reg(SN_TX_WR(socket->index), wr_ptr, 2);
}


static bool fits_socket_txbuf(const struct WIZnetSocket *socket, const size_t frame_size) {
        uint8_t free_space[2];
        eth_read_reg(SN_TX_FSR(socket->index), free_space, 2);
        const uint16_t current_free_space = (free_space[0] << 8) | (free_space[1]);
        if (frame_size <= current_free_space) {
                return true;
        }

        return false;
}

static void write_txbuf(struct WIZnetSocket *socket, const uint8_t *frame, const uint16_t frame_size) {
        uint8_t offsets[2];
        eth_read_reg(SN_TX_WR(socket->index), offsets, 2);
        const uint16_t current_offset = ((offsets[0] << 8) | (offsets[1])) & socket->socket.socket_txbuf_mask;

        if (current_offset + frame_size > socket->socket.socket_txbuf_size_max) {
                const size_t rest_size = socket->socket.socket_txbuf_size_max - current_offset;
                eth_write_reg(socket->txbuf_start + current_offset, frame, rest_size);

                eth_write_reg(socket->txbuf_start, frame + rest_size, frame_size - rest_size);
        }
        else {
                eth_write_reg(socket->txbuf_start + current_offset, frame, frame_size);
        }
}


static int tx_raw_frame(
        struct NetworkInterface *interface, const int socket_number, const char *frame, const size_t frame_size
) {
        struct WIZnetSocket *socket = (struct WIZnetSocket *) interface->sockets[socket_number];
        //TODO: socket validation


        if (!fits_socket_txbuf(socket, frame_size)) {
                return -EMSGSIZE;
        }

        write_txbuf(socket, frame, frame_size);

        set_socket_tx_wr(socket, frame_size);

        socket_send(socket);
        const int status = wait_send_complete(socket);
        return status;
}

static int rx_raw_frame(struct NetworkInterface *interface, int socket_number, char *buffer, size_t length) {
        struct WIZnetSocket *socket = (struct WIZnetSocket *) interface->sockets[socket_number];

        uint8_t ir[1];
        do {
                eth_read_reg(SN_IR(socket->index), ir, 1);

                ir[0] &= RECV;
        } while (!ir[0]);

        if (ir[0] & RECV) {
                constexpr uint8_t clear[] = {RECV};
                eth_write_reg(SN_IR(socket->index), clear, 1);

                //receive process
        }

        return -1;
}

struct NetworkInterface *setup_ethernet_chip(void) {
        setup_eth_spi();

        //reset
        constexpr uint8_t cmd_rst[] = {0x80};
        eth_write_reg(MR, cmd_rst, sizeof(cmd_rst));
        delay_ms(64); //stable time from the datasheet

        if (!is_chip_compatible()) {
                return ERR_PTR(-ENODEV);
        }

        setup_communication_details();

        struct NetworkInterface *interface = (struct NetworkInterface *) kmalloc(sizeof(*interface));
        struct NetworkInterfaceOperations *i_op = (struct NetworkInterfaceOperations *) kmalloc(sizeof(*i_op));
        i_op->setup_interface_information = setup_interface_information;
        i_op->open_socket = open_socket;
        i_op->bind_socket = bind_socket;
        i_op->listen_socket = listen_socket;
        i_op->accept_socket = accept_socket;
        i_op->tx_raw_frame = tx_raw_frame;
        i_op->rx_raw_frame = rx_raw_frame;
        interface->i_op = i_op;

        struct Socket **sockets = (struct Socket **) kmalloc(4 * sizeof(struct Socket *));
        //sockets memory - 4KB per socket (0x02 = 2KB, 0x04 = 4KB, 0x08 = 8KB)
        for (int i = 0; i < 4; i++) {
                constexpr uint8_t cmd[] = {0x04};
                eth_write_reg(SN_TXBUF_SIZE(i), cmd, sizeof(cmd)); // 4KB TX
                eth_write_reg(SN_RXBUF_SIZE(i), cmd, sizeof(cmd)); // 4KB RX

                struct WIZnetSocket *socket = (struct WIZnetSocket *) kmalloc(sizeof(struct WIZnetSocket));
                socket->socket.mode = CLOSED;
                socket->socket.socket_txbuf_mask = 0x0fff;
                socket->socket.socket_txbuf_size_max = 4096;
                socket->txbuf_start = S0_TX_BASE + (i * 4096);
                socket->index = i;
                sockets[i] = (struct Socket *) socket;
        }
        interface->sockets = sockets;

        return interface;
}
