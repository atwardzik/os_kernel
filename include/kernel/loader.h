//
// Created by Artur Twardzik on 28/05/2026.
//

#ifndef OS_LOADER_H
#define OS_LOADER_H

#include <stddef.h>

#include "types.h"

struct ProcessPage {
        void *page_ptr;
        unsigned int pages_count;

        off_t _start_offset;

        //todo: add leftover memory from process pages to reuse in the heap
};

struct ProcessPage *load_exec(void *fbytes);

#endif //OS_LOADER_H
