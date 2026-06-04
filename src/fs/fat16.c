//
// Created by Artur Twardzik on 03/06/2026.
//

#include "fat16.h"

#include "kernel/memory.h"

struct FAT16_BootRecord {
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
} __attribute__((packed));

struct FAT16_SuperBlockOperations {
        struct SuperBlockOperations s_op;

        struct HardDriveOperations hd_op;
};

struct FAT16_SuperBlock {
        struct SuperBlock sb;

        struct FAT16_BootRecord boot_record;
};

struct FAT16_Inode {
        struct VFS_Inode vfs_inode;

        size_t file_size;
        uint16_t first_cluster;
};


static struct InodeOperations *i_op;
static struct FileOperations *i_fop;


static struct VFS_Inode *FAT16_alloc_inode(struct SuperBlock *sb) {
        struct FAT16_Inode *inode = kmalloc(sizeof(*inode));
        memset(inode, 0, sizeof(*inode));
        inode->vfs_inode.i_op = i_op;
        inode->vfs_inode.i_fop = i_fop;
        inode->vfs_inode.i_sb = sb;

        return (struct VFS_Inode *) inode;
}

static void FAT16_destroy_inode(struct VFS_Inode *) {}

static struct Dentry *FAT16_lookup(struct VFS_Inode *parent, struct Dentry *file, unsigned int) {
        const auto sb = (struct FAT16_SuperBlock *) parent->i_sb;
        auto sb_op = (struct FAT16_SuperBlockOperations *) sb->sb.s_op;

        return nullptr;
}

static ssize_t FAT16_read(struct File *file, void *buf, size_t count, off_t file_offset) {
        const struct FAT16_Inode *inode = (struct FAT16_Inode *) file->f_inode;
        const auto sb = (struct FAT16_SuperBlock *) file->f_inode->i_sb;
        const auto sb_op = (struct FAT16_SuperBlockOperations *) sb->sb.s_op;

        const uint16_t bytes_per_sector = sb->boot_record.bytes_per_sector;
        const off_t cluster_offset = file_offset / bytes_per_sector;
        const off_t sector_offset = file_offset % bytes_per_sector;

        size_t block_size = 0;
        if (count % bytes_per_sector == 0) {
                block_size = count;
        }
        else {
                block_size = count - (count % bytes_per_sector) + bytes_per_sector;
        }

        char temp_buf[block_size];
        if (sb_op->hd_op.read_block(inode->first_cluster + cluster_offset, block_size, temp_buf) == 0) {
                memcpy(buf, temp_buf + sector_offset, count);

                file->f_pos += count;
                return count;
        }

        return 0;
}

static struct FAT16_BootRecord read_boot_sector(const uint32_t block_number, const struct HardDriveOperations *hd_op) {
        char buf[512];
        hd_op->read_block(block_number, 512, buf);

        const struct FAT16_BootRecord boot_record = *(struct FAT16_BootRecord *) buf;

        return boot_record;
}

static unsigned int disk_num = 0;

struct Dentry *FAT16_mount(
        struct Dentry *parent_dir, const uint32_t block_number, const struct HardDriveOperations *hd_op
) {
        // static operations struct for future inode use
        i_op = kmalloc(sizeof(*i_op));
        i_op->lookup = FAT16_lookup;
        i_op->create = nullptr;

        i_fop = kmalloc(sizeof(*i_fop));
        i_fop->read = FAT16_read;
        i_fop->write = nullptr;

        //filesystem structure
        struct FAT16_SuperBlockOperations *sb_op = kmalloc(sizeof(*sb_op));
        sb_op->s_op.alloc_inode = FAT16_alloc_inode;
        sb_op->s_op.destroy_inode = FAT16_destroy_inode;
        sb_op->hd_op = *hd_op;

        struct FAT16_SuperBlock *sb = kmalloc(sizeof(*sb));
        sb->sb.name = "fat16";
        sb->sb.s_op = (struct SuperBlockOperations *) sb_op;

        const auto boot_record = read_boot_sector(block_number, hd_op);
        memcpy(&sb->boot_record, &boot_record, sizeof(boot_record));

        struct FAT16_Inode *root_inode = (struct FAT16_Inode *) FAT16_alloc_inode((struct SuperBlock *) sb);
        root_inode->vfs_inode.i_mode = S_IFDIR;
        root_inode->vfs_inode.parent = parent_dir->inode;
        root_inode->vfs_inode.i_fop = i_fop;
        root_inode->vfs_inode.i_op = i_op;
        root_inode->first_cluster = block_number
                                    + boot_record.reserved_sectors
                                    + boot_record.FAT_copies * boot_record.sectors_per_FAT;
        root_inode->vfs_inode.i_size = boot_record.max_root_dentries * 32;

        struct Dentry *root = kmalloc(sizeof(*root));
        root->name = "/";
        root->inode = (struct VFS_Inode *) root_inode;
        root->sb = &sb->sb;

        sb->sb.s_root = root;


        //new Dentry in root
        char name[6] = "disk";
        char num[4] = {};
        strcat(name, itoa((int) disk_num, num, 10));
        disk_num += 1;

        struct Dentry parent_dir_entry = {.name = name, .inode = (struct VFS_Inode *) root_inode};
        parent_dir->inode->i_op->create(parent_dir->inode, &parent_dir_entry, S_IFDIR | 0755);


        return root;
}


int FAT16_decode_entry_name(struct FAT16_DirectoryEntry *entry, char *buf) {
        char name[8] = {};
        memcpy(name, entry->filename, 8);
        char extension[3] = {};
        memcpy(extension, entry->extension, 3);

        int name_last_char = 0;
        for (int i = 0; i < 8; ++i) {
                if (name[i] == 0x20) {
                        break;
                }

                name_last_char += 1;
                buf[i] = name[i];
        }

        if (extension[0] != 0x20) {
                buf[name_last_char] = '.';

                for (int i = 0; i < 3; ++i) {
                        if (extension[i] == 0x20) {
                                break;
                        }

                        name_last_char += 1;
                        buf[name_last_char] = extension[i];
                }
        }

        buf[name_last_char + 1] = 0;

        return 0;
}
