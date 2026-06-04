//
// Created by Artur Twardzik on 03/06/2026.
//

#include "fat16.h"

#include "errno.h"
#include "kernel/memory.h"
#include "kernel/proc.h"

//FIXME: THIS IS ALL HIGHLY UNTESTED FOR BLOCK SIZES OTHER THAN 512

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


static size_t align_block_size(size_t count, size_t bytes_per_sector) {
        if (count % bytes_per_sector == 0) {
                return count;
        }

        return count - (count % bytes_per_sector) + bytes_per_sector;
}

static ssize_t read_root_directory(struct File *file, void *buf, const size_t count, const off_t file_offset) {
        const struct FAT16_Inode *inode = (struct FAT16_Inode *) file->f_inode;
        const auto sb = (struct FAT16_SuperBlock *) file->f_inode->i_sb;
        const auto sb_op = (struct FAT16_SuperBlockOperations *) sb->sb.s_op;

        const uint16_t bytes_per_sector = sb->boot_record.bytes_per_sector;
        const off_t cluster_offset = file_offset / bytes_per_sector;
        const off_t sector_offset = file_offset % bytes_per_sector;

        const size_t block_size = align_block_size(count, bytes_per_sector);

        char *temp_buf = kmalloc(block_size);
        if (sb_op->hd_op.read_block(inode->first_cluster + cluster_offset, block_size, temp_buf) == 0) {
                memcpy(buf, temp_buf + sector_offset, count);

                file->f_pos += count;
                kfree(temp_buf);
                return count;
        }

        kfree(temp_buf);
        return 0;
}

static ssize_t follow_fat_chain(struct File *file, void *buf, const size_t count, const off_t file_offset) {
        const struct FAT16_Inode *inode = (struct FAT16_Inode *) file->f_inode;
        const auto sb = (struct FAT16_SuperBlock *) file->f_inode->i_sb;
        const auto sb_op = (struct FAT16_SuperBlockOperations *) sb->sb.s_op;

        const uint16_t bytes_per_sector = sb->boot_record.bytes_per_sector;
        const off_t cluster_offset = file_offset / bytes_per_sector;
        const off_t sector_offset = file_offset % bytes_per_sector;

        const size_t aligned_size = align_block_size(count, bytes_per_sector);


        const size_t fat_size = sb->boot_record.sectors_per_FAT * bytes_per_sector;
        char *fat = kmalloc(fat_size);
        if (!fat) {
                return -ENOMEM;
        }


        const uint16_t root_cluster = ((struct FAT16_Inode *) sb->sb.s_root->inode)->first_cluster;
        const uint16_t fat_start = root_cluster - (sb->boot_record.FAT_copies * sb->boot_record.sectors_per_FAT);
        if (sb_op->hd_op.read_block(fat_start, fat_size, fat) != 0) {
                return -1; //could not read FAT
        }

        const uint16_t data_region_start =
                root_cluster + (sb->boot_record.max_root_dentries * 32 / bytes_per_sector);
        uint16_t cluster = inode->first_cluster;
        uint16_t fat_entry;
        //determine cluster for current file offset
        for (int i = 0; i < cluster_offset - 1; ++i) { //fixme: boundaries?
                fat_entry = *(uint16_t *) (fat + cluster * 2);
                cluster = fat_entry;
        }

        int i = 0;

        size_t read_bytes = 0;
        do {
                const uint16_t physical_sector =
                        data_region_start + ((cluster - 2) * sb->boot_record.sectors_per_cluster);

                if (sb_op->hd_op.read_block(physical_sector,
                                            sb->boot_record.sectors_per_cluster * bytes_per_sector,
                                            buf + i * bytes_per_sector) != 0
                ) {
                        return -1;
                }
                read_bytes += sb->boot_record.sectors_per_cluster * bytes_per_sector;

                fat_entry = *(uint16_t *) (fat + cluster * 2);
                cluster = fat_entry;
        } while (read_bytes < count && fat_entry >= 3 && fat_entry <= 0xffef);

        file->f_pos += read_bytes;
        return 0;
}

static ssize_t FAT16_read(struct File *file, void *buf, const size_t count, const off_t file_offset) {
        const struct FAT16_Inode *inode = (struct FAT16_Inode *) file->f_inode;
        const auto sb = (struct FAT16_SuperBlock *) file->f_inode->i_sb;

        const uint16_t root_cluster = ((struct FAT16_Inode *) sb->sb.s_root->inode)->first_cluster;
        if (inode->first_cluster == root_cluster) {
                return read_root_directory(file, buf, count, file_offset);
        }

        return follow_fat_chain(file, buf, count, file_offset);
}

static uint16_t FAT16_find_file_start_cluster(
        const char *direntries, const size_t direntries_size, struct Dentry *file
) {
        char name[9] = {};
        memcpy(name, file->name, 8);
        int extension_index = -1;
        for (int i = 0; i < 8; ++i) {
                if (name[i] == '.') {
                        name[i] = 0;
                        extension_index = i;
                }
        }

        char extension[4] = {};
        if (extension_index > 0) {
                memcpy(extension, file->name + extension_index + 1, 3);
                for (int i = 0; i < 3; ++i) {
                        if (extension[i] == 0x20) {
                                extension[i] = 0;
                        }
                }
        }

        int name_length = strlen(name);
        int extension_length = strlen(extension);


        for (size_t i = 0; i < direntries_size / 32; ++i) {
                const auto dirent = (struct FAT16_DirectoryEntry *) (direntries + i * 32);

                if (strncasecmp(name, dirent->filename, name_length) == 0 &&
                    ((!extension[0] && dirent->extension[0] == 0x20) ||
                     strncasecmp(extension, dirent->extension, extension_length) == 0)
                ) {
                        return dirent->first_cluster;
                }
        }

        return 0;
}

static struct Dentry *FAT16_lookup_root_directory(struct FAT16_Inode *inode, struct Dentry *file, unsigned int) {
        struct File file_wrapper = {.f_inode = (struct VFS_Inode *) inode, .f_pos = 0};
        while (file_wrapper.f_pos != inode->vfs_inode.i_size - 1) {
                char buf[512];
                read_root_directory(&file_wrapper, buf, 512, file_wrapper.f_pos);

                uint16_t first_cluster = 0;
                if ((first_cluster = FAT16_find_file_start_cluster(buf, 512, file))) {
                        const auto found = (struct FAT16_Inode *) FAT16_alloc_inode(inode->vfs_inode.i_sb);
                        found->first_cluster = first_cluster;

                        struct Process *current_process = scheduler_get_current_process();
                        add_to_owned_inodes(&current_process->owned_inodes, (struct VFS_Inode *) found);

                        file->inode = (struct VFS_Inode *) found;
                        return file;
                }
        }

        return nullptr;
}

static struct Dentry *FAT16_lookup(struct VFS_Inode *parent, struct Dentry *file, unsigned int) {
        struct FAT16_Inode *inode = (struct FAT16_Inode *) parent;
        const auto sb = (struct FAT16_SuperBlock *) parent->i_sb;
        const auto sb_op = (struct FAT16_SuperBlockOperations *) sb->sb.s_op;

        const uint16_t root_cluster = ((struct FAT16_Inode *) sb->sb.s_root->inode)->first_cluster;
        if (inode->first_cluster == root_cluster) {
                return FAT16_lookup_root_directory(inode, file, 0);
        }

        return nullptr;
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
