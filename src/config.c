//
// Created by Artur Twardzik on 08/04/2026.
//

#include "config.h"

#include "errno.h"

static runtime_config_t kconf_mutable = {
        .io_dev = {
                .uart = {
                        .enabled = true,
                        .baud_rate = 115200
                },

                .vga_display = {
                        .enabled = true
                },

                .ps2_keyboard = {
                        .enabled = true
                }
        },

        .network = {
                .enabled = true,
                .ip = {192, 168, 2, 1},
                .gateway = {192, 168, 1, 1},
                .subnet_mask = {255, 255, 255, 0}
        },

        .debug = {
                .log_level = LOG_DEBUG,
                .context_switch_history = true
        },
};

const runtime_config_t *const kconf = &kconf_mutable;

int load_kernel_config(const struct VFS_Inode *inode) {
        if (!inode) {
                return -ENOENT;
        }

        struct File file = {
                .f_op = inode->i_fop,
                .f_inode = inode,
                .f_pos = 0,
        };

        char buf[64];
        // TODO: add config file parsing
        inode->i_fop->read(&file, buf, 1, 0);

        return 0;
}
