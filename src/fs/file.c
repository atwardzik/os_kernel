//
// Created by Artur Twardzik on 27/08/2025.
//

#include "fs/file.h"

#include "tty.h"
#include "kernel/proc.h"

#include <stdio.h>


int sys_open(const char *name, int flags, int mode) { return -1; }

int sys_close(int file) { return -1; }

int sys_read(int file, char *ptr, int len) {
        struct Process const *current_process = scheduler_get_current_process();

        if (file >= current_process->files.count || file < 0) {
                sys_write(1, "[!] There is no such file descriptor\n", 37);
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
                sys_write(1, "[!] There is no such file descriptor\n", 37);
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

int ksys_write(const int file, char *ptr, const int len) {
        if (file != 1 && file != 2) {
                sys_write(1, "[!] There is no such file descriptor\n", 37);
                __asm__("bkpt   #0");
                return 0;
        }

        for (int i = 0; i < len; i++) {
                write_byte(*ptr++);
        }

        return len;
}

int sys_write(const int file, char *ptr, const int len) {
        struct Process const *current_process = scheduler_get_current_process();

        if (file >= current_process->files.count || file < 0) {
                ksys_write(1, "[!] There is no such file descriptor\n", 37); //write directly to tty...
                __asm__("bkpt   #0");
                return 0;
        }

        struct File *current_file = current_process->files.fdtable[file];
        if (current_file->f_op->write) {
                return current_file->f_op->write(current_file, ptr, len, current_file->f_pos);
        }

        return -1;
}
