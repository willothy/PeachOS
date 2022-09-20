#ifndef MEMORY_H_
#define MEMORY_H_

#include <stddef.h>

void* memset(void* ptr, int c, size_t size);

void* memcpy(void* restrict dest, const void* restrict src, size_t n);

void* memmove(void* dest, const void* src, size_t n);

int memcmp(void* s1, void* s2, int count);

#endif