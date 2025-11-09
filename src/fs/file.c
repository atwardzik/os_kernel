//
// Created by Artur Twardzik on 27/08/2025.
//

#include "fs/file.h"

#include "tty.h"
#include "kernel/memory.h"
#include "kernel/proc.h"

#include <string.h>
#include <sys/_default_fcntl.h>

static struct VFS_Inode *get_parent(const char *name) {
        const struct Process *current_process = scheduler_get_current_process();
        struct VFS_Inode *inode = current_process->root;
        if (strcmp(name, "/") == 0) {
                return inode;
        }

        char *path = kmalloc(sizeof(name));
        strcpy(path, name);
        const char *token = strtok(path, "/");
        while (token) {
                struct Dentry dentry = {
                        .name = token,
                };

                struct Dentry *result = inode->i_op->lookup(inode, &dentry, 0);
                if (!result) {
                        kfree(path);
                        return nullptr;
                }

                inode = result->inode;
                kfree(result);
                token = strtok(NULL, "/");
        }
        kfree(path);


        return inode;
}

static struct VFS_Inode *get_file(struct VFS_Inode *parent, const char *name) {
        struct File parent_handler = {
                .f_inode = parent,
                .f_op = parent->i_fop,
        };

        char *buf = kmalloc(sizeof(char) * parent->i_size);

        parent_handler.f_op->read(&parent_handler, buf, parent->i_size, 0);

        size_t offset = 0;
        while (offset < parent->i_size) {
                const size_t record_size = ((struct DirectoryEntry *) buf + offset)->rec_len;
                if (!record_size) {
                        break;
                }

                struct DirectoryEntry file_dentry;
                for (size_t i = 0; i < record_size; ++i) {
                        *((char *) (&file_dentry) + i) = buf[offset];
                        offset += 1;
                }

                if (strcmp(file_dentry.name, name) == 0) {
                        kfree(buf);
                        return parent->i_sb->inode_table[file_dentry.inode_index];
                }
        }

        kfree(buf);
        return nullptr;
}

static struct VFS_Inode *create_file(struct VFS_Inode *parent, const char *name, uint32_t mode) {
        struct Dentry file = {
                .name = name,
        };

        parent->i_op->create(parent, &file, mode);
        return parent->i_sb->inode_table[parent->i_sb->current_inode_count - 1];
}

static int get_file_descriptor(struct VFS_Inode *inode, const char *name) {
        struct Process *current_process = scheduler_get_current_process();

        struct File *found_file = kmalloc(sizeof(*found_file));
        found_file->f_op = inode->i_fop;
        found_file->f_inode = inode;
        found_file->f_pos = 0;
        found_file->f_path = name; //TODO: it's not a valid path
        current_process->files.fdtable[current_process->files.count] = found_file;
        current_process->files.count += 1;

        return current_process->files.count - 1;
}

static char *get_path(const char *name) {
        const char *last_file = strrchr(name, '/');
        if (!last_file) {
                return nullptr;
        }

        char *path = kmalloc(sizeof(name));

        size_t i = 0;
        while (name + i != last_file) {
                path[i] = name[i];

                i += 1;
        }
        path[i] = 0;

        return path;
}

int sys_open(const char *name, int flags, int mode) {
        char *path = get_path(name);
        struct VFS_Inode *parent = nullptr;
        const char *filename;
        if (path) {
                parent = get_parent(path);
                filename = strrchr(name, '/') + 1;
                kfree(path);
        }
        else {
                parent = scheduler_get_current_process()->root;
                filename = name;
        }
        struct VFS_Inode *file = get_file(parent, filename);

        if (!file && flags & O_CREAT) {
                const uint16_t file_mode = (flags & O_DIRECTORY) ? S_IFDIR | 0666 : 0777; //TODO: change permissions
                file = create_file(parent, filename, file_mode);
        }


        if (file) {
                return get_file_descriptor(file, filename);
        }

        return -1;
}

int sys_close(int file) { return -1; }

int ksys_write(const int file, char *ptr, const int len) {
        if (file != 1 && file != 2) {
                ksys_write(1, "[!] There is no such file descriptor\n", 37);
                __asm__("bkpt   #0");
                return 0;
        }

        for (int i = 0; i < len; i++) {
                write_byte(*ptr++);
        }

        return len;
}

int sys_read(int file, char *ptr, int len) {
        struct Process const *current_process = scheduler_get_current_process();

        if (file >= current_process->files.count || file < 0) {
                ksys_write(1, "[!] There is no such file descriptor\n", 37);
                __asm__("bkpt   #0");
                return 0;
        }

        struct File *current_file = current_process->files.fdtable[file];
        if (current_file->f_op->read) {
                return current_file->f_op->read(current_file, ptr, len, current_file->f_pos);
        }

        return -1;
}


/**
 * Instead of blocking processes it spinlocks on keyboard
 */
int ksys_read(int file, char *ptr, int len) {
        if (file != 0) {
                ksys_write(1, "[!] There is no such file descriptor\n", 37);
                __asm__("bkpt   #0");
                return 0;
        }

        void *stream_start = get_current_keyboard_buffer_offset();
        int stream_offset = 0;
        while (true) {
                stream_offset = newline_buffered_at();

                if (stream_offset) {
                        break;
                }
        }

        int offset = 0;
        while (offset < len && offset < stream_offset) {
                *(ptr + offset) = *(char *) (stream_start + offset);

                offset += 1;
        }

        return offset;
}

int sys_write(const int file, char *ptr, const int len) {
        struct Process const *current_process = scheduler_get_current_process();

        if (file >= current_process->files.count || file < 0) {
                ksys_write(1, "[!] There is no such file descriptor\n", 37);
                __asm__("bkpt   #0");
                return 0;
        }

        struct File *current_file = current_process->files.fdtable[file];
        if (current_file->f_op->write) {
                return current_file->f_op->write(current_file, ptr, len, current_file->f_pos);
        }

        return -1;
}
