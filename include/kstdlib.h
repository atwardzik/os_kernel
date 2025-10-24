//
// Created by Artur Twardzik on 24/10/2025.
//

#ifndef OS_KSTDLIB_H
#define OS_KSTDLIB_H

#include "types.h"

#include <stdint.h>

static inline char *itoa(int value, char *const str, const int base) {
        if (value == 0) {
                *str = '0';
                return str;
        }

        int i = 0;
        int temp_value = value;
        while (temp_value) {
                i += 1;
                temp_value /= base;
        }

        while (value) {
                const uint8_t digit = (value % base);
                if (digit > 9) {
                        *(str + i - 1) = 'A' + 10 - digit;
                }
                else {
                        *(str + i - 1) = digit + 0x30;
                }

                value /= base;
                i -= 1;
        }

        return str;
}

static inline bool isprint(const char c) {
        if (c >= 0x20 && c <= 0xff) {
                return true;
        }

        return false;
}

#endif //OS_KSTDLIB_H
