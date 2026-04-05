//
// Created by Artur Twardzik on 01/04/2026.
//

#include "network.h"
#include "libc.h"
#include "errno.h"
#include "memory.h"

static constexpr size_t ETHERNET_HEADER_LENGTH = 14;

int str2mac(const char *src_mac, char *buf) {
        if (!src_mac || !buf) {
                return -EINVAL;
        }
        if (strlen(src_mac) != 17) {
                return -EINVAL;
        }

        char mac[18];
        strcpy(mac, src_mac);

        const char *byte = strtok(mac, ":");
        int i = 0;
        while (i < 6) {
                if (strlen(byte) != 2) {
                        return -EINVAL;
                }

                buf[i] = strtoul(byte, nullptr, 16);

                byte = strtok(nullptr, ":");
                i += 1;
        }

        return 0;
}

int str2ip(const char *src_ip, char *buf) {
        if (!src_ip || !buf) {
                return -EINVAL;
        }
        if (strlen(src_ip) > 15) {
                return -EINVAL;
        }

        char ip[18];
        strcpy(ip, src_ip);

        const char *byte = strtok(ip, ".");
        int i = 0;
        while (i < 4) {
                if (strlen(byte) > 3) {
                        return -EINVAL;
                }
                const unsigned int octet = strtoul(byte, nullptr, 10);
                if (octet > 255) {
                        return -EINVAL;
                }

                buf[i] = octet;

                byte = strtok(nullptr, ".");
                i += 1;
        }

        return 0;
}


void setup_network_information(
        struct NetworkInterface *interface, const char *ip_address, const char *mac_address, const char *gateway,
        const char *subnet_mask
) {
        char mac[6];
        if (str2mac(mac_address, mac) == 0) {
                memcpy(interface->mac, mac, sizeof(mac));
        }

        char gw[4];
        if (str2ip(gateway, gw) == 0) {
                memcpy(interface->gateway, gw, sizeof(gw));
        }

        char sub[4];
        if (str2ip(subnet_mask, sub) == 0) {
                memcpy(interface->subnet_mask, sub, sizeof(sub));
        }

        char ip[4];
        if (str2ip(ip_address, ip) == 0) {
                memcpy(interface->ip, ip, sizeof(ip));
        }

        interface->i_op->setup_interface_information(interface);
}

int send_raw_frame(
        struct NetworkInterface *interface, int socket_number, const char *src_mac, const char *dst_mac,
        const uint16_t ether_type, const char *data, const size_t data_length
) {
        char src[6];
        char dst[6];
        str2mac(src_mac, src);
        str2mac(dst_mac, dst);

        const size_t frame_size = ETHERNET_HEADER_LENGTH + data_length;
        char *frame = (char *) kmalloc(frame_size);

        memcpy(frame, src, 6);
        memcpy(frame + 6, dst, 6);
        frame[12] = ether_type >> 8;
        frame[13] = ether_type & 0xff;
        memcpy(frame + 14, data, data_length);

        const int ret = interface->i_op->tx_raw_frame(interface, socket_number, frame, frame_size);
        kfree(frame);
        return ret;
}
