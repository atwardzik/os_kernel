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

typedef int pid_t;
typedef struct SpawnFileActions spawn_file_actions_t;
typedef struct SpawnAttr spawnattr_t;
struct stat;
typedef char *caddr_t;
typedef typeof(void (int)) *sighandler_t;

void exit(int code);

int write(int file, const void *buf, int len);

int read(int file, void *buf, int len);

int open(const char *name, int flags, int mode);

int close(int file);

int fstat(int file, struct stat *st);

int readdir(int dirfd, struct DirectoryEntry *directory_entry);

int chdir(const char *path);

int lseek(const int file, int offset, int whence);

char *getcwd(char *buf, unsigned int len);

pid_t spawnp(
        void (*process_entry_ptr)(void),
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
);

pid_t spawn(
        int fd,
        const spawn_file_actions_t *file_actions,
        const spawnattr_t *attrp,
        char *const argv[],
        char *const envp[]
);

int kill(int pid, int sig);

void sigreturn(void);

sighandler_t signal(int signum, sighandler_t handler);

pid_t wait(int *stat_loc);

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

int printf(const char *format, ...);

void *memset(void *dest, int ch, unsigned int count);

void *memcpy(void *dest, const void *src, unsigned int count);

#endif // LIBC_H
