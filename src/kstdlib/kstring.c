//
// Created by Artur Twardzik on 11/11/2025.
//

#include "kstring.h"

#include <string.h>

int kstrcmp(const char *str1, const char *str2) {
        return 1;
}

char *kstrtok(char *str, const char *delim) {
        static char *token_start = nullptr;
        static char *ptr = nullptr;
        if (str) {
                ptr = str;
        }

        if (!ptr || !*ptr) {
                return nullptr;
        }

        token_start = ptr;
        ptr += strcspn(ptr, delim);

        const size_t delims_len = strspn(ptr, delim);
        for (size_t i = 0; i < delims_len; ++i) {
                *ptr = 0;
                ptr += 1;
        }

        return token_start;
}
