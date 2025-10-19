//
// Created by Artur Twardzik on 27/08/2025.
//

#ifndef OS_FILE_H
#define OS_FILE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


struct Inode {};

constexpr int MAX_OPEN_FILE_DESCRIPTORS = 8;

struct FileOperations;

struct File {
        void *f_path;
        const struct FileOperations *f_op;
        unsigned int f_flags;
        off_t f_pos;
#if 0
        spinlock_t f_lock;

        struct Path f_path;
#endif
};

struct FileOperations {
        /**
         * size_t lseek(struct File *file, size_t offset, int whence);
         */
        ssize_t (*lseek)(struct File *, size_t, int);

        /**
         * ssize_t read(struct File *file, void buf, size_t count, off_t file_offset);
         */
        ssize_t (*read)(struct File *, void *, size_t, off_t);

        /**
         * ssize_t write(struct File *file, const void buf, size_t count, off_t file_offset);
         */
        ssize_t (*write)(struct File *, void *, size_t, off_t);

        int (*open)(struct Inode *, struct File *);

        int (*flush)(struct File *);

#if 0
        int (*lock)(struct File *, int, struct File_lock *);

        int (*check_flags)(int);

        void (*show_fdinfo)(struct seq_File *m, struct File *f);
#endif
};

struct Files {
        size_t count;
        struct File **fdtable;
        // struct File *next;
};

typedef struct fs fs_t;

typedef enum fs_object_type {
        ENTRY_TYPE_DIR,
        ENTRY_TYPE_FILE,
} fs_object_type_t;

typedef struct fs_object fs_object_t;

#if 0
fs_t *fs_init(void);

void fs_deinit(fs_t *fs);

fs_object_t *f_get_parent(fs_t *fs, const char *path);

fs_object_t *f_get_entry(fs_t *fs, const char *path);

char *f_get_name(const fs_object_t *entry);

char *get_path(const fs_object_t *entry);

int is_dir(const fs_object_t *entry);

int is_file(const fs_object_t *entry);

fs_object_t *f_create(fs_t *fs, const char *path, int flags);

fs_object_t *f_open(fs_t *fs, const fs_object_t *entry, unsigned int flags);

void f_close(fs_object_t *fh);

ssize_t f_read(fs_object_t *fh, char *buf, size_t len);

ssize_t f_write(fs_object_t *fh, const char *buf, size_t len);

ssize_t f_seek(fs_object_t *fh, off_t offset, int mode);

size_t f_tell_current_position(const fs_object_t *fh);

int f_unlink(fs_object_t *entry);

int f_rename(fs_t *fs, const char *src, const char *dst);

fs_object_t *opendir(fs_t *fs, const fs_object_t *entry);

void closedir(fs_object_t *dh);

const fs_object_t *readdir(fs_object_t *dh);

void seekdir(fs_object_t *dh, long loc);

long telldir(fs_object_t *dh);

fs_object_t *mkdir(fs_t *fs, const char *name);

int rmdir(fs_object_t *entry);
#endif


void init_file_descriptors(void);

int sys_open(const char *name, int flags, int mode);

int sys_close(int file);

int sys_read(int file, char *ptr, int len);

int sys_write(int file, char *ptr, int len);

#endif // OS_FILE_H
