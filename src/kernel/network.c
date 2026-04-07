//
// Created by Artur Twardzik on 01/04/2026.
//

#include "network.h"
#include "errno.h"
#include "error.h"
#include "libc.h"
#include "memory.h"
#include "proc.h"
#include "socket.h"
#include "drivers/ethernet.h"

static constexpr size_t MAX_INTERFACES_COUNT = 1;

static constexpr size_t ETHERNET_HEADER_LENGTH = 14;

// struct Sockfs_Inode {
//         struct VFS_Inode vfs_inode;
//
//         struct Socket *socket;
// };

static struct {
        struct NetworkInterface **interfaces;

        // struct Sockfs_Inode *sockfs_root;
} network_manager __attribute__((section(".data")));

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

//TODO: make it ioctl
static void setup_network_information(
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

        interface->setup_network_information(interface);
}

int send_raw_frame(
        int sockfd,
        const char *src_mac, const char *dst_mac,
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

        sys_write(sockfd, frame, frame_size);
        kfree(frame);
        return -1;
}


int init_network(void) {
        struct NetworkInterface **interfaces = kmalloc(MAX_INTERFACES_COUNT * sizeof(*interfaces));
        if (!interfaces) {
                return -ENOMEM;
        }

        printf("\x1b[96;40m[!] Checking network adapter: \x1b[0m");
        struct NetworkInterface *eth0 = init_ethernet();
        if (IS_ERR(eth0)) {
                printf("\x1b[91;40mNot found or adapter incompatible\x1b[0m\n");
                return -ENETDOWN;
        }
        else {
                printf("\x1b[92;40m Ok\x1b[0m\n");
                interfaces[0] = eth0;
        }

        printf("\x1b[96;40m[!] Setting up network adapter: \x1b[0m");

        // TODO: dhcp protocol
        const char *ip_addr = "192.168.2.1";
        const char *mac_addr = "de:ad:01:10:be:ef";
        setup_network_information(eth0,
                                  ip_addr,
                                  mac_addr,
                                  "192.168.2.1",
                                  "255.255.255.0"
        );

        printf("\x1b[92;40m Ok\x1b[0m\n");
        printf("\x1b[96;40m[!] Network adapter set up to:\x1b[0m %s %s\n", ip_addr, mac_addr);

        network_manager.interfaces = interfaces;

        return 0;
}

int sys_socket(int domain, int type, int protocol) {
        if (domain != AF_INET) {
                UNIMPLEMENTED("Protocol family unknown. Currently only ipv4 (AF_INET) is supported.");
        }

        struct Process *current_process = scheduler_get_current_process();
        if (current_process->files.count >= MAX_OPEN_FILE_DESCRIPTORS) {
                sys_write(2, "[!] Too much files opened.\n", 28);
                __asm__("bkpt   #0");
                return -1;
        }

        // TODO: make it dependent on network interfaces connected
        struct Socket *socket = nullptr;
        int res;
        switch (type) {
                case SOCK_STREAM:
                        socket = network_manager.interfaces[0]->create_socket();
                        if (IS_ERR(socket)) {
                                return -ENOSOCKFREE;
                        }
                        res = socket->s_op->open(socket, TCP);
                        break;
                case SOCK_RAW:
                        socket = network_manager.interfaces[0]->create_raw_socket();
                        if (IS_ERR(socket)) {
                                return -ENOSOCKFREE;
                        }
                        res = socket->s_op->open(socket, MACRAW);
                        break;
                default:
                        UNIMPLEMENTED("Unknown type. Currently only TCP and RAW supported.");
        }

        if (res < 0) {
                return res;
        }

        for (size_t i = 0; i < MAX_OPEN_FILE_DESCRIPTORS; ++i) {
                if (current_process->files.fdtable[i] == nullptr) {
                        current_process->files.fdtable[i] = &socket->file;
                        current_process->files.count += 1;

                        return i;
                }
        }

        return -ENOSOCKFREE; //should not happen?
}

int sys_bind(int sockfd, const struct sockaddr *addr, size_t addrlen) {
        return 0;
}

int sys_listen(int sockfd, int backlog) {
        return 0;
}

int sys_accept(int sockfd, struct sockaddr *addr, size_t addrlen) {
        return 0;
}

int sys_connect(int sockfd, const struct sockaddr *addr, size_t adrlen) {
        return 0;
}
