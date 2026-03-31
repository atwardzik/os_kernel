//
// Created by Artur Twardzik on 30/03/2026.
//

#ifndef OS_ETHERNET_H
#define OS_ETHERNET_H

#include <stdint.h>
#include <stddef.h>

enum SocketMode {
        CLOSED = 0,
        TCP    = 1,
        UDP    = 2,
        IPRAW  = 3,
        MACRAW = 4,
};


struct IPAddr {
        uint32_t address;
};

struct MACAddr {
        uint64_t address;
};

int setup_ethernet_chip(void);

void setup_network_information(
        const char *ip_address, const char *mac_address, const char *gateway, uint32_t subnet_mask
);

int open_socket(int socket_number, enum SocketMode mode);

int send_raw_frame(const char *src_mac, const char *dst_mac, uint16_t ether_type, const char *data);

#endif //OS_ETHERNET_H
