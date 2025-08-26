//
// Created by Artur Twardzik on 21/08/2025.
//

#ifndef OS_CTYPE_H
#define OS_CTYPE_H

static inline bool isprint(const char c) {
        if (c >= 0x20 && c <= 0xff) {
                return true;
        }

        return false;
}

#endif //OS_CTYPE_H