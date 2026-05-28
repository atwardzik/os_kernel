//
// Created by Artur Twardzik on 28/05/2026.
//

#ifndef OS_LOADER_H
#define OS_LOADER_H

#include <stddef.h>

struct ProcessPage {
        void *page_ptr;
        unsigned int pages_count;

        void *_start_address;

        //todo: add leftover memory from process pages to reuse in the heap
};

struct ProcessPage *load_exec(void *fbytes);

#endif //OS_LOADER_H
