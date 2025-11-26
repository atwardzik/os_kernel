//
// Created by Artur Twardzik on 22/11/2025.
//
#include "libc.h"

int main(int argc, char *argv[]) {
        if (argc < 2) {
                dprintf(2, "Not enough parameters\n\tUsage: %s [-ne] [file]\n", argv[0]);

                return -1;
        }

        bool LINE_NUMBERING = false;
        bool NEWLINE_CHARS_PRINTING = false;
        char c;
        while ((c = getopt(argc, argv, "ne")) != (char) -1) {
                switch (c) {
                        case 'n':
                                LINE_NUMBERING = true;
                                break;
                        case 'e':
                                NEWLINE_CHARS_PRINTING = true;
                                break;
                        case '?':
                        default:
                                printf("-%c - unknown parameter\n", c);
                                break;
                }
        }

        const char *file = nullptr;
        file = argv[argc - 1];

        if (!file) {
                dprintf(2, "No file specified\n");

                return -1;
        }

// #ifdef DEBUG
        printf("Line numbering is %s\n", LINE_NUMBERING ? "on" : "off");
        printf("Printing newline chars is %s\n", NEWLINE_CHARS_PRINTING ? "on" : "off");
        printf("File is: %s\n", file);
// #endif

        int fd = open(file, O_RDONLY, 0);

        unsigned long file_len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        int bytes_read = 0;
        int i = 1;
        do {
                char buf[2] = {};
                bytes_read = read(fd, buf, 1);

                if (buf[0] == '\n' && NEWLINE_CHARS_PRINTING) {
                        write(1, "$", 1);
                }

                write(1, buf, 1);

                if (buf[0] == '\n' && LINE_NUMBERING) {
                        // dummy solution until printf supports fixed positions
                        int j = 10000;
                        while (j > i) {
                                puts(" ");

                                j /= 10;
                        }

                        printf("%i  ", i);
                        i += 1;
                }
        } while (bytes_read);

        close(fd);
        return 0;
}
