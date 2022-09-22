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

int strnlen_terminator(const char* str, int max, char terminator) {
    int len = 0;
    while (str[len] && str[len] != terminator && len < max)
        len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    while (*src)
        *dest++ = *src++;
    *dest = '\0';
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest + strlen(dest);
    while (*src != '\0')
        *ptr++ = *src++;

    *ptr = '\0';
    return dest;
}

int istrncmp(const char* str1, const char* str2, int n) {
    unsigned char u1, u2;
    while (n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;

        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;

        if (u1 == '\0')
            return 0;
    }
    return 0;
}

int strncmp(const char* str1, const char* str2, int n) {
    unsigned char u1, u2;

    while (n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

bool is_digit(char c) { return c >= 48 && c <= 57; }
bool is_alpha(char c) { return (c >= 65 && c <= 90) || (c >= 97 && c <= 122); }
bool is_whitespace(char c) {
    return (c >= 9 && c <= 13) || c == 32 || c == 160;
}

int to_numeric_digit(char c) { return c - 48; }
char* to_numeric_string(int num) {
    static char str[12];
    str[11] = '\0';
    int i = 10;
    bool is_negative = num < 0;
    if (is_negative)
        num = -num;
    do {
        str[i--] = (num % 10) + 48;
        num /= 10;
    } while (num > 0);
    if (is_negative)
        str[i--] = '-';
    return &str[i + 1];
}

char tolower(char c) {
    if (c >= 65 && c <= 90)
        return c + 32;
    return c;
}

char toupper(char c) {
    if (c >= 97 && c <= 122)
        return c - 32;
    return c;
}

char* to_hex_string(unsigned long num) {
    static char hex[9];
    hex[0] = '0';
    hex[1] = 'x';
    hex[10] = '\0';
    for (int i = 9; i >= 2; i--) {
        int digit = num & 0xF;
        if (digit < 10) {
            hex[i] = digit + 48;
        } else {
            hex[i] = digit + 55;
        }
        num >>= 4;
    }
    return hex;
}
