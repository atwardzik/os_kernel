//
// Created by Artur Twardzik on 25/08/2025.
//

#ifndef OS_STDLIB_H
#define OS_STDLIB_H

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

#endif //OS_STDLIB_H
