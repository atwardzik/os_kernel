//
// Created by Artur Twardzik on 03/06/2026.
//

#ifndef OS_FAT16_H
#define OS_FAT16_H

#include "file.h"

#include <stdint.h>


struct Dentry *FAT16_mount(
        struct Dentry *parent_dir, uint32_t block_number, const struct HardDriveOperations *hd_op
);


#endif //OS_FAT16_H
