//
// Created by Artur Twardzik on 30/03/2026.
//

#ifndef OS_ETHERNET_H
#define OS_ETHERNET_H

#include <stdint.h>
#include <stddef.h>

enum ETH_STATUS {
        ETH_OK,
        ETH_ERR,
        ETH_CHIP_INCOMPATIBLE,
};

struct IPAddr {
        uint32_t address;
};

struct MACAddr {
        uint64_t address;
};

int setup_ethernet(const char *ip_address, const char *mac_address, const char *gateway, uint32_t subnet_mask);

#endif //OS_ETHERNET_H
