//
// Created by Artur Twardzik on 24/10/2025.
//

#ifndef OS_KSTDIO_H
#define OS_KSTDIO_H

#include "../types.h"

#include <stdarg.h>

int kprintf(const char *, ...);

char *kfgets(char *, int);

#endif //OS_KSTDIO_H
