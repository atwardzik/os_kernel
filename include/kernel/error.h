//
// Created by Artur Twardzik on 01/04/2026.
//

#ifndef OS_ERROR_H
#define OS_ERROR_H

#define UNIMPLEMENTED(msg) kernel_panic("Unimplemented feature: " msg, \
                                        __FILE__, __LINE__, __func__)

static inline void *ERR_PTR(long error) {
        return (void *) error;
}

void kernel_panic(const char *msg, const char *file, int line, const char *func);

#endif //OS_ERROR_H
