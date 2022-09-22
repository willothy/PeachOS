#include "memory/memory.h"

void* memset(void* ptr, int c, size_t size) {
    char* c_ptr = (char*)ptr;
    for (size_t i = 0; i < size; i++) {
        c_ptr[i] = c;
    }
    return ptr;
}

// C99 standard prototype
void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    const char* sp = src;
    char* dp = dest;

    while (n--)
        *dp++ = *sp++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* temp[n];
    memcpy(temp, src, n);
    memcpy(dest, temp, n);
    return dest;
}

int memcmp(void* s1, void* s2, int count) {
    char* c1 = s1;
    char* c2 = s2;

    while (count-- > 0) {
        if (*c1++ != *c2++)
            return c1[-1] < c2[-1] ? -1 : 1;
    }
    return 0;
}