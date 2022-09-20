#include "string/string.h"

int strlen(const char* str) {
    int len = 0;
    while (str[len])
        len++;
    return len;
}

int strnlen(const char* str, int max) {
    int len = 0;
    while (str[len] && len < max)
        len++;
    return len;
}

void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

bool is_digit(char c) { return c >= 48 && c <= 57; }

int to_numeric_digit(char c) { return c - 48; }