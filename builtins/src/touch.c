//
// Created by Artur Twardzik on 22/11/2025.
//
#include "libc.h"

int main(int argc, char *argv[]) {
        if (argc < 2) {
                puts("No argument specified");
                return -1;
        }

        int fd = open(argv[1], O_CREAT, 0);
        if (fd > 0) {
                close(fd);
                return 0;
        }

        return -1;
}
