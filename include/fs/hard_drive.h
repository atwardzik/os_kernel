//
// Created by Artur Twardzik on 03/06/2026.
//

#ifndef OS_HARD_DRIVE_H
#define OS_HARD_DRIVE_H

#include <stddef.h>
#include <stdint.h>

struct HardDriveOperations {
        int (*read_block)(uint32_t block_number, size_t block_size, char *buffer, size_t bufsize);

        int (*write_block)(uint32_t block_number, size_t block_size, char *buffer);

        int (*deinit)(void);
};

#endif //OS_HARD_DRIVE_H
