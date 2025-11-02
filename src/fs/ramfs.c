//
// Created by Artur Twardzik on 01/11/2025.
//

#include "ramfs.h"

#include "kernel/memory.h"

#include <string.h>

struct Dentry *ramfs_mount(
        const char *source, const char *target_dir, const char *filesystemtype, unsigned long mountflags
) {
        struct SuperBlockOperations *fs_op = kmalloc(sizeof(*fs_op));
        fs_op->alloc_inode = &ramfs_alloc_inode;
        fs_op->destroy_inode = &ramfs_destroy_inode;

        struct SuperBlock *fs = kmalloc(sizeof(*fs) + 32 * sizeof(void *));
        fs->name = "ramfs";
        fs->s_op = fs_op;

        fs->current_inode_count = 0;
        fs->max_inode_count = 32;

        struct VFS_Inode *root_inode = fs->s_op->alloc_inode(fs);
        struct Dentry *root = kmalloc(sizeof(*root));
        root->name = "/";
        root->inode_index = 0;
        root->sb = fs;

        fs->s_root = root;

        return root;
}

struct VFS_Inode *ramfs_alloc_inode(struct SuperBlock *sb) {
        if (sb->current_inode_count >= sb->max_inode_count - 1) {
                return nullptr;
        }

        struct InodeOperations *i_op = kmalloc(sizeof(*i_op));
        i_op->lookup = &ramfs_lookup;
        i_op->create = &ramfs_create_file;
        i_op->mkdir = &ramfs_mkdir;

        struct FileOperations *i_fop = kmalloc(sizeof(*i_fop));
        i_fop->read = ramfs_read;
        i_fop->write = ramfs_write;


        struct RAMFS_Inode *inode = kmalloc(sizeof(*inode));
        inode->vfs_inode.i_op = i_op;
        inode->vfs_inode.i_fop = i_fop;
        inode->vfs_inode.i_sb = sb;
#if 0
        inode->vfs_inode.i_uid = uid;
        inode->vfs_inode.i_gid = gid;
        inode->vfs_inode.i_mode = (file_type << 16) | permissions;
        inode->vfs_inode.i_ctime = time(nullptr);
        inode->vfs_inode.i_links_count = 0;
        inode->vfs_inode.i_size = 0;
#endif

        inode->file_begin = nullptr;

        sb->inode_table[sb->current_inode_count] = inode;
        sb->current_inode_count += 1;

        return (struct VFS_Inode *) inode;
}

void ramfs_destroy_inode(struct VFS_Inode *) {
        //
}


struct Dentry *ramfs_lookup(struct VFS_Inode *parent, struct Dentry *file, unsigned int) {
        // *children = kmalloc(sizeof(struct Dentry *) * parent->children_count);
        // *children_count = parent->children_count;
        //
        // for (size_t i = 0; i < parent->children_count; ++i) {
        //         (*children)[i] = parent->children[i];
        // }

        return nullptr;
}


int ramfs_create_file(struct VFS_Inode *parent, struct Dentry *new_file, uint16_t permissions) {
        struct SuperBlock *fs = parent->i_sb;

        struct VFS_Inode *new_inode = fs->s_op->alloc_inode(parent->i_sb);
        if (!new_inode) {
                return -1;
        }
        new_inode->i_mode = permissions;

        new_file->inode_index = fs->current_inode_count - 1;

        //update parent contents

        const uint16_t rec_len = sizeof(struct DirectoryEntry) + strlen(new_file->name) + 1;
        struct DirectoryEntry *directory_entry = kmalloc(rec_len);
        directory_entry->rec_len = rec_len;
        directory_entry->inode_index = new_file->inode_index;
        directory_entry->file_type = '-';
        strcpy(directory_entry->name, new_file->name);

        struct File parent_handler = {
                .f_inode = parent,
        };
        parent->i_fop->write(&parent_handler, directory_entry, directory_entry->rec_len, parent->i_size);

        return 0;
}

struct Dentry *ramfs_mkdir(struct VFS_Inode *parent, struct Dentry *new_dir, mode_t permissions) {
        // new_dir->max_children_count = 8;
        // new_dir->children = kmalloc(sizeof(struct Dentry *) * 8); //TODO: REPLACE TO DYNAMIC

        return new_dir;
}

ssize_t ramfs_write(struct File *file, void *buf, size_t count, off_t file_offset) {
        const size_t current_size = file->f_inode->i_size;
        struct RAMFS_Inode *inode_ptr = (struct RAMFS_Inode *) file->f_inode;

        if (!inode_ptr->bytes_allocated) {
                inode_ptr->file_begin = kmalloc(512);
                inode_ptr->bytes_allocated = 512;
        }
        else if (file_offset + count > inode_ptr->bytes_allocated) {
                // inode_ptr->file_begin = krealloc(inode_ptr->file_begin, current_size + 512); // +relocate!!!
        }

        for (int i = 0; i < count; i++) {
                *(char *) (inode_ptr->file_begin + file_offset + i) = *(char *) (buf++);
        }

        inode_ptr->vfs_inode.i_size += count;

        return count;
}

ssize_t ramfs_read(struct File *file, void *buf, size_t count, off_t file_offset) {
        struct RAMFS_Inode *inode_ptr = (struct RAMFS_Inode *) file->f_inode;
        void *ptr = inode_ptr->file_begin;

        int offset = 0;
        while (offset < count && offset < inode_ptr->bytes_allocated) {
                *(char *) (buf + offset) = *(char *) (ptr + offset);

                offset += 1;
        }

        return offset;
}

struct File *ramfs_get_file_handler(struct Dentry *file, unsigned int flags) {
        struct FileOperations *std_fop = kmalloc(sizeof(*std_fop));
        std_fop->read = &ramfs_read;
        std_fop->write = &ramfs_write;

        struct File *f_std = kmalloc(sizeof(*f_std));
        f_std->f_pos = 0;
        f_std->f_op = std_fop;
        f_std->f_owner = 0;
        f_std->f_inode = file->sb->inode_table[file->inode_index];

        return f_std;
}
