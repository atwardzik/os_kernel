//
// Created by Artur Twardzik on 03/06/2026.
//

#include "fat16.h"

#include "kernel/memory.h"

static unsigned int disk_num = 0;

struct Dentry *FAT16_mount(
        struct Dentry *parent_dir, const char *target_dir, const char *filesystemtype, unsigned long mountflags
) {
        struct SuperBlockOperations *sb_op = kmalloc(sizeof(*sb_op));
        sb_op->alloc_inode = nullptr;
        sb_op->destroy_inode = nullptr;

        struct SuperBlock *sb = kmalloc(sizeof(*sb));
        sb->name = "fat16";
        sb->s_op = sb_op;

        sb->current_inode_count = 0;
        sb->max_inode_count = 32;
        sb->inode_table = kmalloc(sizeof(void *) * sb->max_inode_count);

        struct VFS_Inode *root_inode = parent_dir->inode;
        root_inode->i_mode |= S_IFDIR;
        struct Dentry *root = kmalloc(sizeof(*root));
        root->name = "/";
        root->inode = root_inode;
        root->sb = sb;

        sb->s_root = root;


        //new Dentry in root
        char name[6] = "disk";
        char num[4] = {};
        strcat(name, itoa((int) disk_num, num, 10));
        disk_num += 1;

        struct Dentry parent_dir_entry = {.name = name};

        parent_dir->inode->i_op->create(parent_dir->inode, &parent_dir_entry, S_IFDIR | 0755);

        return root;
}
