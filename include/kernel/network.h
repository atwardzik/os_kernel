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

struct SocketOperations;

struct Socket {
        enum SocketMode mode;
        uint16_t port;

        size_t socket_txbuf_size_max;
        unsigned int socket_txbuf_mask;

        size_t socket_rxbuf_size_max;
        unsigned int socket_rxbuf_mask;

        const struct SocketOperations *s_op;
};

struct SocketOperations {
        int (*open)(struct Socket *socket, enum SocketMode mode);

        int (*bind)(struct Socket *socket, uint16_t port);

        int (*listen)(struct Socket *socket);

        int (*accept)(struct Socket *socket);

        int (*recv)(struct Socket *socket, char *buffer, size_t length);

        int (*send)(struct Socket *socket, const char *frame, size_t frame_size);

        int (*close)(struct Socket *socket);

        int (*connect)(struct Socket *socket, const char *ipaddr, uint16_t port);
};

struct NetworkInterface {
        char mac[6];
        char ip[4];
        char gateway[4];
        char subnet_mask[4];


        int (*setup_network_information)(struct NetworkInterface *interface);

        struct Socket *(*create_socket)(void);

        struct Socket *(*create_raw_socket)(void);

        // void (*setup_socket_file_operations)()
};

//TODO: make it ioctl
void setup_network_information(
        struct NetworkInterface *interface,
        const char *ip_address,
        const char *mac_address,
        const char *gateway,
        const char *subnet_mask
);

int send_raw_frame(
        struct Socket *socket,
        const char *src_mac,
        const char *dst_mac,
        uint16_t ether_type,
        const char *data,
        size_t data_length
);

int str2mac(const char *src_mac, char *buf);

int str2ip(const char *src_ip, char *buf);


int init_network(void);

struct sockaddr {
        int i;
};

int sys_socket(int domain, int type, int protocol);

int sys_bind(int sockfd, const struct sockaddr *addr, size_t addrlen);

int sys_listen(int sockfd, int backlog);

int sys_accept(int sockfd, struct sockaddr *addr, size_t addrlen);

int sys_connect(int sockfd, const struct sockaddr *addr, size_t adrlen);

#endif //OS_NETWORK_H
