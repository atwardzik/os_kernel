//
// Created by Artur Twardzik on 08/04/2026.
//

#ifndef OS_CONFIG_H
#define OS_CONFIG_H

#include "fs/file.h"

#include <stdint.h>

typedef struct {
        struct {
                struct {
                        bool enabled;
                        uint32_t baud_rate;
                } uart;

                struct {
                        bool enabled;
                } vga_display;

                struct {
                        bool enabled;
                        unsigned int clk_pin;
                        unsigned int dat_pin;
                } ps2_keyboard;
        } io_dev;

        struct {
                bool enabled;

                uint8_t ip[4];
                uint8_t gateway[4];
                uint8_t subnet_mask[4];
        } network;

        struct {
                enum {
                        LOG_NONE, LOG_DEBUG
                } log_level;

                bool context_switch_history;
        } debug;
} runtime_config_t;

extern const runtime_config_t *const kconf;

int load_kernel_config(const struct VFS_Inode *inode);

#endif //OS_CONFIG_H
