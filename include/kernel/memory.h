//
// Created by Artur Twardzik on 30/12/2024.
//

#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#include <stdint.h>

extern uint8_t __heap_start__[];
extern uint8_t __heap_end__[];

constexpr int kernel_heap_end_offset = 4096;
static uint8_t *const user_space_heap_start_ptr = __heap_start__ + kernel_heap_end_offset;
static uint8_t *const heap_end_ptr = __heap_end__;

void *kmalloc(size_t size);

void *krealloc(void *ptr, size_t new_size);

void kfree(void *ptr);

size_t get_allocated_size(void);

size_t get_current_heap_size(void);

// void memset(void *dst, int value, size_t count);

void *kmemcpy(void *dst, const void *src, size_t count);

#endif //MEMORY_H
