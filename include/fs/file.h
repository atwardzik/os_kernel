//
// Created by Artur Twardzik on 27/08/2025.
//

#ifndef OS_FILE_H
#define OS_FILE_H

#include "types.h"

#include <stdint.h>
#include <time.h>

/*
 * /home/user/foo.txt
 *       ├── Directory "/home/user/"
 *       │     └── entry: ("foo.txt" → inode 12345)
 *       ├── Inode 12345
 *       │     ├── size = 1024 bytes
 *       │     ├── owner = UID 1000
 *       │     ├── block pointers = [42, 43]
 *       └── Data blocks
 *             ├── Block 42: first 512 bytes of the file
 *             └── Block 43: next 512 bytes
 */

/*
 *      [Superblock]
 *      [Inode Table]
 *         ├── inode 1 (root directory pointer)
 *         ├── inode 2 (etc pointer)
 *         ├── inode 3 (foo.txt pointer)
 *      [Data Blocks]
 *         ├── (directories, file contents)
 */

#define O_DIRECTORY 0x200000

// Bits:   [15........12][11....9][8....6][5....3][2....0]
//            file type   special   user    group   other
#define	S_IFREG 0100000
#define	S_IFDIR 0040000
#define	S_IFCHR 0020000
#define	S_IFBLK 0060000
#define	S_IFLNK 0120000
#define	S_IFSOCK 0140000
#define	S_IFIFO 0010000

constexpr int MAX_OPEN_FILE_DESCRIPTORS = 8;

struct SuperBlock;

struct VFS_Inode;

struct Dentry;

struct FileOperations;

struct File;

enum FileType {
        REGULAR_FILE,
        DIRECTORY,
};


struct SuperBlockOperations {
        struct VFS_Inode *(*alloc_inode)(struct SuperBlock *sb);

        void (*destroy_inode)(struct VFS_Inode *);

        void (*free_inode)(struct VFS_Inode *);
};

struct SuperBlock {
        const char *name;
        const struct SuperBlockOperations *s_op;
        struct Dentry *s_root;

        // struct Dentry *(*mount)();

        size_t max_inode_count;
        size_t current_inode_count;

        void *inode_table[] __attribute__((counted_by(max_inode_count)));
};


struct Dentry {
        const char *name;
        struct VFS_Inode *inode;

        struct SuperBlock *sb;
};

constexpr size_t MAX_FILENAME_LEN = 32;

struct DirectoryEntry {
        uint8_t file_type;
        uint32_t inode_index;
        // uint16_t rec_len;
        char name[MAX_FILENAME_LEN];
};

struct InodeOperations {
        struct Dentry *(*lookup)(struct VFS_Inode *, struct Dentry *, unsigned int);

        int (*create)(struct VFS_Inode *, struct Dentry *, uint16_t);

        // create with specific flag
        // struct Dentry *(*mkdir)(struct VFS_Inode *, struct Dentry *, mode_t);
};

struct VFS_Inode {
        uint16_t i_mode; // File type and permissions
        uint16_t i_uid;
        uint16_t i_gid;
        uint32_t i_flags; // Compressed; append only; don't update access time etc.

        const struct InodeOperations *i_op;
        const struct FileOperations *i_fop;
        struct SuperBlock *i_sb;

        struct VFS_Inode *parent;

        off_t i_size;
        time_t i_atime;
        time_t i_ctime;
        time_t i_mtime;
        uint32_t i_generation; // File Version
        uint32_t i_blocks;     // Total number or 512-bytes blocks reserved to contain the data of this inode

        uint16_t i_links_count; // How many times this inode is linked (referred to)
};

struct File {
        void *f_path;
        struct VFS_Inode *f_inode;
        const struct FileOperations *f_op;
        unsigned int f_flags;
        off_t f_pos;
        pid_t f_owner;

        // struct Path f_path;
};

struct FileOperations {
        ssize_t (*lseek)(struct File *file, size_t offset, int whence);

        ssize_t (*read)(struct File *file, void *buf, size_t count, off_t file_offset);

        ssize_t (*write)(struct File *file, void *buf, size_t count, off_t file_offset);

        int (*open)(struct VFS_Inode *, struct File *);

        int (*flush)(struct File *);
};

struct Files {
        size_t count;
        struct File **fdtable;
        // struct File *next;
};


void init_file_descriptors(void);

int sys_open(const char *name, int flags, int mode);

int sys_close(int file);

int sys_read(int file, char *ptr, int len);

int sys_write(int file, char *ptr, int len);

int sys_readdir(int dirfd, struct DirectoryEntry *directory_entry);

int sys_chdir(const char *path);

int sys_lseek(const int file, off_t offset, int whence);

#endif // OS_FILE_H
