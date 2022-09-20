#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

int strlen(const char* ptr);
int strnlen(const char* ptr, int max);
void strcpy(char* dest, const char* src);

bool is_digit(char c);
int to_numeric_digit(char c);

#endif // STRING_H
