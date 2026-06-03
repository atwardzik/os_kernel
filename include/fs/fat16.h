//
// Created by Artur Twardzik on 03/06/2026.
//

#ifndef OS_FAT16_H
#define OS_FAT16_H

#include "file.h"

#include <stdint.h>

struct FAT16BootRecord {
        uint8_t jump_code[3];
        const char oem_name[8];
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluster;
        uint16_t reserved_sectors;
        uint8_t FAT_copies;
        uint16_t max_root_dentries;
        uint16_t small_part_sectors_count;
        uint8_t media_descriptor;
        uint16_t sectors_per_FAT;
        uint16_t sectors_per_track;
        uint16_t heads_count;
        uint32_t hidden_sectors;
        uint32_t large_part_sectors_count;
        uint16_t drive_number;
        uint8_t extended_boot_signature;
        uint32_t volume_serial_number;
        const char volume_label[11];
        const char fs_type[8];
        uint8_t bootstrap_code[448];
        uint8_t boot_sector_signature;
};

struct Dentry *FAT16_mount(
        struct Dentry *parent_dir, const char *target_dir, const char *filesystemtype, unsigned long mountflags
);


#endif //OS_FAT16_H
