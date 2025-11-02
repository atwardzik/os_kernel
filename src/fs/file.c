//
// Created by Artur Twardzik on 27/08/2025.
//

#include "fs/file.h"

#include "tty.h"
#include "kernel/memory.h"
#include "kernel/proc.h"

#include <string.h>


int sys_open(const char *name, int flags, int mode) {
        struct Process *current_process = scheduler_get_current_process();

        // usage read first file in directory
        struct File parent_handler = {
                .f_inode = current_process->root,
                .f_op = current_process->root->i_fop,
        };

        char *buf = kmalloc(sizeof(char) * current_process->root->i_size);

        parent_handler.f_op->read(&parent_handler, buf, current_process->root->i_size, 0);

        size_t offset = 0;
        while (offset < current_process->root->i_size) {
                size_t record_size = ((struct DirectoryEntry *) (buf))->rec_len;
                struct DirectoryEntry *file_dentry = kmalloc(record_size);

                for (size_t i = 0; i < record_size; ++i) {
                        *((char *) (file_dentry) + i) = buf[offset];
                        offset += 1;
                }

                if (strcmp(file_dentry->name, name) != 0) {
                        kfree(file_dentry);
                        continue;
                }

                auto file_inode =
                        (struct VFS_Inode *) current_process->root->i_sb->inode_table[file_dentry->inode_index];

                //new file descriptor
                struct File *found_file = kmalloc(sizeof(*found_file));
                found_file->f_op = file_inode->i_fop;
                found_file->f_inode = file_inode;
                found_file->f_pos = 0;
                found_file->f_path = name; //TODO: it's not a valid path
                current_process->files.fdtable[current_process->files.count] = found_file;
                current_process->files.count += 1;

                kfree(file_dentry);
                kfree(buf);
                return current_process->files.count - 1;
        }

        kfree(buf);
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
