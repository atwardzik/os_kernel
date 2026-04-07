//
// Created by Artur Twardzik on 01/04/2026.
//

#include "network.h"
#include "errno.h"
#include "libc.h"
#include "memory.h"
#include "proc.h"

static constexpr size_t ETHERNET_HEADER_LENGTH = 14;

struct Sockfs_Inode {
        struct VFS_Inode vfs_inode;

        struct Socket *socket;
};

static struct {
        struct NetworkInterface **interfaces;

        struct Sockfs_Inode *sockfs_root;
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

        interface->setup_network_information(interface);
}

int send_raw_frame(
        struct Socket *socket, const char *src_mac, const char *dst_mac,
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

        socket->s_op->send(socket, frame, frame_size);
        kfree(frame);
        return -1;
}


int init_network(void) {
        const auto interfaces = (struct NetworkInterface **) kmalloc(2 * sizeof(struct NetworkInterface *));
        if (!interfaces) {
                return -ENOMEM;
        }

        network_manager.interfaces = interfaces;

        // const struct Dentry *sockfs = sockfs_mount(nullptr, nullptr, nullptr, 0);

        // network_manager.sockfs_root = sockfs->inode;

        return 0;
}

static int recv(struct File *file, void *buf, size_t count, off_t) {
        struct NetworkInterface *interface = network_manager.interfaces[0];
        //TODO: dynamic interfaces depending on needs


        // return interface->i_op->rx_raw_frame(interface, 0, buf, count);
        return 0;
}

static int send(struct File *, void *buf, size_t count, off_t) {
        struct NetworkInterface *interface = network_manager.interfaces[0];
        //TODO: dynamic interfaces depending on needs

        return -1;
}

int sys_socket(int domain, int type, int protocol) {
        struct Dentry tty_dentry = {
                .name = "new_socket", //TODO: DYNAMIC NAMING
        };

        // network_manager.sockfs_root->i_op->create(network_manager.sockfs_root, &tty_dentry, S_IFCHR | 0666);
        // struct Dentry *tty = network_manager.sockfs_root->i_op->lookup(network_manager.sockfs_root, &tty_dentry, 0);

        // tty->inode->i_fop;

        // get parent process and assign new file descriptor
        // socket must be a file?

        return 0;
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

