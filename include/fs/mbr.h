//
// Created by Artur Twardzik on 03/06/2026.
//

#ifndef OS_MBR_H
#define OS_MBR_H

#include "hard_drive.h"

#include <stdint.h>
#include <stddef.h>

struct PartitionTableEntry {
        uint8_t attributes;
        uint8_t chs_start[3];
        uint8_t type;
        uint8_t chs_end[3];
        uint32_t lba_start;
        uint32_t sector_count;
} __attribute__((packed));

struct PartitionTableEntry *get_mbr_partition_table(struct HardDriveOperations *hd_op);

#endif //OS_MBR_H
