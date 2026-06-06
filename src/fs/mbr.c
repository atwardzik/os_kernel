//
// Created by Artur Twardzik on 03/06/2026.
//

#include "mbr.h"

#include "libc.h"

#include "kernel/memory.h"

struct PartitionTableEntry *get_mbr_partition_table(struct HardDriveOperations *hd_op) {
        char buffer[512] = {};
        hd_op->read_block(0, 512, buffer);

        if (buffer[510] != 0x55 || buffer[511] != 0xaa) {
                return nullptr;
        }

        struct PartitionTableEntry *partition_table = kmalloc(sizeof(*partition_table) * 4);
        memcpy(partition_table, buffer + 0x1be, 4 * 16);

        return partition_table;
}
