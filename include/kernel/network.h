//
// Created by Artur Twardzik on 01/04/2026.
//

// Networking is quite hard to implement - if multiple processes need internet
// access the OS would have to manage the sockets. The chip on the board is
// WIZnet W5100S which has 3 hardwired tcp/udp sockets and one MACRAW socket.
//
// The only possibility for the OS to manage processes' sockets (virtual - as
// used in normal unix programming) is to use MACRAW mode and to implement the
// whole TCP/UDP stack in the OS.
//
// Therefore, for the sake of simplicity and quicker development, for now we
// assume that not much processes will use the ethernet. By that about 3
// connections are meant.
//
// TODO: implement TCP stack and connection management at the OS level.

#ifndef OS_NETWORK_H
#define OS_NETWORK_H

#include <stddef.h>
#include "types.h"
#include "fs/file.h"

enum SocketMode {
        CLOSED = 0,
        TCP    = 1,
        UDP    = 2,
        IPRAW  = 3,
        MACRAW = 4,
};

struct SocketOperations;

struct Socket {
        struct File file;

        enum SocketMode mode;
        uint16_t port;

        const struct SocketOperations *s_op;
};

struct SocketOperations {
        int (*open)(struct Socket *socket, enum SocketMode mode);

        int (*bind)(struct Socket *socket, uint16_t port);

        int (*listen)(struct Socket *socket);

        int (*accept)(struct Socket *socket, struct sockaddr *addr, size_t addrlen);

        int (*connect)(struct Socket *socket, struct sockaddr *addr, size_t addrlen);
};

struct NetworkInterface {
        char mac[6];
        char ip[4];
        char gateway[4];
        char subnet_mask[4];


        int (*setup_network_information)(struct NetworkInterface *interface);

        struct Socket *(*create_socket)(void);

        struct Socket *(*create_raw_socket)(void);

        int (*destroy_socket)(struct Socket *socket);
};

int str2mac(const char *src_mac, char *buf);

int str2ip(const char *src_ip, char *buf);


int init_network(void);


int sys_socket(int domain, int type, int protocol);

int sys_bind(int sockfd, const struct sockaddr *addr, size_t addrlen);

int sys_listen(int sockfd, int backlog);

int sys_accept(int sockfd, struct sockaddr *addr, size_t addrlen);

int sys_connect(int sockfd, const struct sockaddr *addr, size_t addrlen);

#endif //OS_NETWORK_H
