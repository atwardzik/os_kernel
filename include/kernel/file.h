//
// Created by Artur Twardzik on 27/08/2025.
//

#ifndef OS_FILE_H
#define OS_FILE_H

void init_file_descriptors(void);

int sys_open(const char *name, int flags, int mode);

int sys_close(int file);

int sys_read(int file, char *ptr, int len);

int sys_write(int file, char *ptr, int len);

#endif //OS_FILE_H
