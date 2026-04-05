//
// Created by Artur Twardzik on 01/04/2026.
//

#ifndef OS_NETWORK_H
#define OS_NETWORK_H

#include <stddef.h>
#include "types.h"

enum SocketMode {
        CLOSED = 0,
        TCP    = 1,
        UDP    = 2,
        IPRAW  = 3,
        MACRAW = 4,
};

struct Socket {
        enum SocketMode mode;
        uint16_t port;

        size_t socket_txbuf_size_max;
        unsigned int socket_txbuf_mask;
};

struct NetworkInterfaceOperations;

struct NetworkInterface {
        char mac[6];
        char ip[4];
        char gateway[4];
        char subnet_mask[4];

        struct Socket **sockets;
        const struct NetworkInterfaceOperations *i_op;
};

struct NetworkInterfaceOperations {
        int (*setup_interface_information)(struct NetworkInterface *interface);

        int (*open_socket)(
                struct NetworkInterface *interface,
                int socket_number,
                enum SocketMode mode
        );

        int (*bind_socket)(
                struct NetworkInterface *interface,
                int socket_number,
                uint16_t port
        );

        int (*listen_socket)(
                struct NetworkInterface *interface,
                int socket_number
        );

        int (*accept_socket)(
                struct NetworkInterface *interface,
                int socket_number
        );

        int (*rx_raw_frame)(
                struct NetworkInterface *interface,
                int socket_number,
                char *buffer,
                size_t length
        );

        int (*tx_raw_frame)(
                struct NetworkInterface *interface,
                int socket_number,
                const char *frame,
                size_t frame_size
        );
};

void setup_network_information(
        struct NetworkInterface *interface,
        const char *ip_address,
        const char *mac_address,
        const char *gateway,
        const char *subnet_mask
);

int send_raw_frame(
        struct NetworkInterface *interface,
        int socket_number,
        const char *src_mac,
        const char *dst_mac,
        uint16_t ether_type,
        const char *data,
        size_t data_length
);

int str2mac(const char *src_mac, char *buf);

int str2ip(const char *src_ip, char *buf);

#endif //OS_NETWORK_H
