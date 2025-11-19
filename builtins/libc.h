//
// Created by Artur Twardzik on 19/11/2025.
//

#ifndef LIBC_H
#define LIBC_H

#define O_BINARY 0x10000
#define O_RDONLY 0
#define O_WRONLY 1
#define SEEK_SET 0
#define O_DIRECTORY 0x200000

constexpr int MAX_FILENAME_LEN = 32;

struct DirectoryEntry {
        unsigned char file_type;
        int inode_index;
        char name[MAX_FILENAME_LEN];
};

void __attribute__((naked)) _start();

/*
 * Syscalls
 */

int write(int file, const void *buf, int len);

int read(int file, void *buf, int len);

int open(const char *name, int flags, int mode);

int close(int file);

int readdir(int dirfd, struct DirectoryEntry *directory_entry);

int chdir(const char *path);

int lseek(const int file, int offset, int whence);

char *getcwd(char *buf, unsigned int len);

/*
 * string
 */

int strlen(const char *str);

int puts(const char *str);

int strcspn(const char *str, const char *delims);

int strspn(const char *str, const char *src);

char *strtok(char *str, const char *delim);

int strcmp(const char *s1, const char *s2);

char *itoa(int value, char *str, int base);

#endif // LIBC_H
