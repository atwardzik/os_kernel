//
// Created by Artur Twardzik on 19/10/2025.
//


#ifndef OS_EXT2_FS_H
#define OS_EXT2_FS_H

#include <stdint.h>
//[+]  - ext specific
//[#]  - inherited from vfs_inode

struct Ext2Inode {
        uint16_t i_mode;        // [#] File type and permissions (default all permissions)
        uint16_t i_uid;         // [#] Owner user ID
        uint32_t i_size;        // [#] File size in bytes (lower 32 bits)
        uint32_t i_atime;       // [#] Last access time
        uint32_t i_ctime;       // [#] Last inode change time
        uint32_t i_mtime;       // [#] Last modification time
        uint32_t i_dtime;       // [+] Delete time
        uint16_t i_gid;         // [#] POSIX group having access to this file
        uint16_t i_links_count; // How many times this inode is linked (referred to)
        uint32_t i_blocks;      // [#] Total number of 512-bytes blocks reserved to contain the data of this inode
        uint32_t i_flags;       // [+] How ext2 implementation should behave when accessing the data for this inode
        uint32_t i_osd1;        // OS dependant value (for linux reserved)

        // 15*32-bit numbers pointing to the blocks containing the data for this inode
        uint32_t i_block[12]; // [+] Direct pointers to data blocks
        uint32_t i_indirect;  // [+] Pointer to single indirect block
        uint32_t i_dindirect; // [+] Pointer to double indirect block
        uint32_t i_tindirect; // [+] Pointer to triple indirect block
        //

        uint32_t i_generation; // [#] File version
        uint32_t i_file_acl;   // [+] Block number containing extended attributes
        uint32_t i_dir_acl;    // [+] 0 if not a regular file (i.e. block devices, directories, etc.)
        uint32_t i_faddr;      // [+] Location of file fragment (always 0)
        struct {
                uint8_t l_i_frag;      // [+] Fragment number (always 0)
                uint8_t l_i_fsize;     // [+] Fragment size (always 0)
                uint16_t l_i_uid_high; // high 16bit of user_id
                uint16_t l_i_gid_high; // high 16bit of group_id
        } i_osd2;
};

#endif //OS_EXT2_FS_H
